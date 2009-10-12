/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#include <unistd.h>
#include <time.h>

#include <iostream>
#include <algorithm>

#include "Client.h"
#include "MailDatabase.h"
#include "MailRecord.h"
#include "LogSink.h"
#include "Options.h"


/*
 * TODO: A mail could transfer between All Mail and Trash, two different primary boxes. Our app would remove it from the first and then redownload for the second -> improve?
 * 			-> first check spam & trash
 * TODO: A lot of error checking, e.g. if you can open the database-file, have write permissions, etc...
 * TODO: Check what happens when an external client (i.e. webmail) marks a mail(link) as "read". Is it still found? Is the mark removed when updating?
 * TODO: GMail accepts 10 simultaneous connections (MAILSTREAM), try to use them to check multiple boxes at the same time
 * 				-> create a simultaneous connection by calling mail_open() without an opened stream to recycle (i.e. mail_open(NULL, mb, options); )
 * TODO: Add error handling for create_hard_link
 * TODO: When implementing the feature to sync back to gmail, check all links of a mail for one that contains "S" for read. If one contains it
		change all links to read (add "S") to emulate GMail's Label implementation. One could create a daemon that continually checks
		all mails for changes like this to then rename all links.

Errors:
	Logging in...(0 times)
	mm_log: %[ALERT] Web login required (Failure)
	Logging in...(1 times)
	mm_log: %[ALERT] Web login required (Failure)
	Logging in...(2 times)
	mm_log: %[ALERT] Web login required (Failure)
	mm_log: ?Too many login failures
*/

/* Sinds we gelijk ni veel krijgen als er een error gebeurd, hier een lijstje
 * met errors:
 * 
 * mail_uid: returned 0 als stroom dood is
 * mail_fetch_envelope NIL als er fout gebeurd is. (PS: fetched "fast" informatie ook)
 * mail_fetch_header: ""
 * mail_fetch_text: ""
 * 
 * 
 */
 
void show_version()
{
	cout << "Gmail Carbon Copy v0.2.x - released July 2009" << endl;
	cout << "http://code.crowdway.com/gmailcc" << endl;
	cout << "Copyright (c) 2009 David Verhasselt (david@crowdway.com)" << endl;
	cout << "PCB-free" << endl;
	cout << endl;
	cout << "Compiled at " << __TIMESTAMP__ << " using g++ " << __VERSION__ << endl;
}
	
 
void finalize(Client& client, MailDatabase& maildb, char* error = NULL)
{
	if (error != NULL)
		Log::error << "Finalizing with error: " << error << Log::endl;

	client.disconnect();	
	maildb.save();
	
	if (error != NULL)
		abort();	
}
 

int main(int argc, char* argv[])
{
	// Start logger
	Log::set_priority(0);

	
	// Check configuration
	Options options(argc, argv);
	
	string username;
	string password;
	string maildir;
	
	if (options.get_version()) {
		show_version(); return 1;
	}
	
	if (options.get_help())	{
		options.show_help(); return 1;
	}
	
	username = options.get_username();
	password = options.get_password();
	maildir = options.get_maildir_path();

	if (options.incomplete()) {
		if (isatty(fileno(stdin)) && isatty(fileno(stdout)))		// If input and output is a terminal, we can still ask the user for values
		{
			if (username.empty()) {
				cout << "Gmail username: ";
				cin >> username;
			}
			
			if (password.empty()) {
				char* pass = getpass("Gmail password: ");			// Deprecated, find another way
				password = pass;
				memset(pass, '*', strlen(pass));					// Clear memory
				delete pass;
			}
			
			if (maildir.empty()) {
				cout << "Target maildir: ";
				cin >> maildir;
			}
			
			cout << endl;
		}
		else
		{
			options.show_help();
			return 1;
		}
	}
	
	// Configure logger
	Log::set_priority(options.get_loglevel());
	Log::set_logfile(options.get_logfile().c_str());
	
	// Start timer
	time_t start_time = time(NULL);	
	Log::info << "Started @ " << ctime(&start_time) << Log::endl;
 
 	// Variables
	Client client;
	unsigned long cacheindex = 0;
	int cacheinterval = 1000;
	char* cachestring = new char[50];
	MailBox* mb;
	MailRecord* mr;
	ENVELOPE* envelope;
	MESSAGECACHE* msgcache;
	
	// Load local database
	MailDatabase* maildb = MailDatabase::load(maildir);

	// Connect to IMAP server
	try {
		client.connect(username, password);
	} catch (InvalidAuthClientException &e) {
		Log::critical << "Unable to log-in to server because of invalid credentials. Please check your username and password." << Log::endl;
		finalize(client, *maildb);
	} catch (WebAuthClientException &e) {
		Log::critical << "Unable to log-in to server because Gmail requires a one-time web login. Please use your browser to login once." << Log::endl;
		finalize(client, *maildb);
	} catch(ClientException &e) {
		Log::critical << "Unable to log-in to server because of an unknown reason. Please check your internet connection." << Log::endl;
		finalize(client, *maildb);
	}
	
	Log::info << "Connection succesfully established. Log-in succesful." << Log::endl;
	
	
	// Load primary mailboxes
	client.open_mailbox("[Gmail]/All Mail");
	mb = maildb->add_mailbox("[Gmail]/All Mail");
	
	
	bool new_mail;
	bool deleted_mail;
	bool refresh_uids = client.uid_validity != mb->uid_validity;
	
	unsigned long uid;
	char* body_data; string body;	
	long unsigned int body_length;
	
	mb->uid_validity = client.uid_validity;
	mb->next_uid = client.uid_next;
	mb->messagecount = client.get_mailcount();

	Log::info << "Mailbox has " << client.get_mailcount() << " messages." << Log::endl;
	
	for(client.msg_index = 1; client.msg_index <= client.get_mailcount(); client.msg_index++) {		
		// Check if we need to cache them
		if (cacheindex < client.msg_index)
		{
			sprintf(cachestring, "%lu:%lu", cacheindex+1, cacheindex+cacheinterval);
			Log::info << "Caching status of mails " << cacheindex+1 << " to " << min(cacheindex+cacheinterval, client.get_mailcount()) << "." << Log::endl;
			
			cacheindex = min(cacheindex+cacheinterval, client.get_mailcount());			
			
			mail_gc (client.stream, GC_ELT);					// Garbage collect the old ones
			mail_fetchfast(client.stream, cachestring);			// We'll need to check the flags either way		
		}
		
		Log::info << "Mail " << client.msg_index << " of " << client.get_mailcount();
				
		mr = NULL;
		
		uid = mail_uid(client.stream, client.msg_index);
		if (uid == 0) finalize(client, *maildb, "(new mail) mail_uid is 0");	// Connection broken			
	
		if (!refresh_uids) {

			mr = maildb->get_mail(uid);		// Try to load the email from our local database using the UID (long)
		}
		
		if (mr == NULL) {	// If UID is invalid or it's a new mail
			Log::info << ", uid unknown";
			envelope = mail_fetchenvelope(client.stream, client.msg_index);			// First fetch the mail metadata
			
			if (envelope == NIL || client.fatal_error) finalize(client, *maildb, "Envelope is NIL");							// Connection broken
			
			if ((envelope->message_id == NULL) || (envelope->message_id[0] == '\0')) {
				Log::error << ", mail has empty message-id. Ignore." << Log::endl;
				continue;
			}
			
			mr = maildb->get_mail(envelope->message_id);							// Again, try to load the email from our local database, this time using the message ID (string)
			
			if (mr == NULL)	// New mail
			{
				Log::info << ", message id unknown. Download." << Log::endl;
			
				body.clear();
				body_data = mail_fetchbody_full(client.stream, client.msg_index, "", &body_length, FT_PEEK | FT_INTERNAL);
				body.append(body_data, body_length);				// Convert to string using the content_length because of possible
																	// binary data inside (and thus also /0 characters.		
				if (client.fatal_error) finalize(client, *maildb, "Fatal Error while downloading body.");
				if (!body.size()) Log::info << " -- An empty mail, how quaint." << Log::endl;				
				
				mr = maildb->new_mail(envelope->message_id, uid, body);
			}
			else			// Existing mail
			{
				Log::info << ", message id known. ";
				
				if (refresh_uids)
				{
					Log::info << "Set uid." << Log::endl;
					mr = maildb->new_mail(envelope->message_id, mb, uid);		// Just update the UID
				}
				else
				{
					Log::info << "Duplicate: ignore." << Log::endl;
				}
			
			}
		}		
		else
		{
			Log::info << ", uid known." << Log::endl;			
		}
			
			
			mail_gc (client.stream, GC_TEXTS);		// TODO: Maybe move this to the gc envelopes statement to also 
													// execute periodically?
													
				
			// Garbage collect envelopes
			if (!(client.msg_index % 20)) {
			//	Log::info << "Collect garbage (envelopes)" << Log::endl;
				mail_gc (client.stream, GC_ENV); }
		
		msgcache = mail_elt(client.stream, client.msg_index);
		
		if (msgcache == NIL || client.fatal_error)  finalize(client, *maildb, "mail_elt returned NIL");	// Connection broken
		
		mr->set_flag_draft(msgcache->draft == 1);
		mr->set_flag_flagged(msgcache->flagged == 1);
		mr->set_flag_passed(msgcache->recent == 1);			// Not sure of this mapping here
		mr->set_flag_replied(msgcache->answered == 1);
		mr->set_flag_seen(msgcache->seen == 1);
		mr->set_flag_trashed(msgcache->deleted == 1);
		
		mr->sync_flags();
		
		// Set touched to true to signify this mail shouldn't be deleted
		mr->mark();
	}
	
	mail_gc (client.stream, GC_ELT);
	
	// Also check trash & spam here
		
	maildb->sweep();

					
	// Load secondary mails
	client.get_mailboxen();	
	for (vector<string>::iterator it = client.mailboxlist.begin(); it < client.mailboxlist.end(); it++)
	{
		Log::info << "Checking mailbox " << *it << "..." << Log::endl;
		
		if (it->compare("[Gmail]/All Mail") == 0)
			continue;
			
		if (it->compare("[Gmail]/Trash") == 0)
			continue;
			
		mb = maildb->add_mailbox(*it);	
		client.open_mailbox(mb->name);
		
		new_mail = client.uid_next != mb->next_uid;
		deleted_mail = new_mail || (client.get_mailcount() < mb->messagecount);		// If there's new mail, there's no way of knowing presearch if there are any deleted messages.
		refresh_uids = client.uid_validity != mb->uid_validity;
		
		Log::info << "This mailbox has possibly ";
		if (!new_mail) Log::info << "no "; Log::info << "new mail, ";
		if (!deleted_mail) Log::info << "no "; Log::info << "deleted mail, and ";
		if (!refresh_uids) Log::info << "no "; Log::info << "refreshed UID's" << Log::endl; 
		
		mb->uid_validity = client.uid_validity;
		mb->next_uid = client.uid_next;
		mb->messagecount = client.get_mailcount();
		
		Log::info << "Checking " << client.get_mailcount() << " messages." << Log::endl;
		
		
		if (new_mail || deleted_mail || refresh_uids)
		{		
			for(client.msg_index = 1; client.msg_index <= client.get_mailcount(); client.msg_index++)
			{
				mr = NULL;
				
				if (!refresh_uids)
				{
					mr = maildb->get_mail(mb, mail_uid(client.stream, client.msg_index));
				}
				
				if (mr == NULL)		// If UID is invalid or it's a new mail
				{
					envelope = mail_fetchenvelope(client.stream, client.msg_index);
					mr = maildb->get_mail(envelope->message_id);
					if (mr != NULL)		// mr should never be NULL, except if something happened between primary and secondary update
						mr = maildb->new_mail(envelope->message_id, mb, mail_uid(client.stream, client.msg_index));		// Just updates UID
					else
						continue;
				}
			
				// Set touched to true to signify this link shouldn't be deleted
				if (mr && deleted_mail)
					mr->find_link(mb)->mark();
			}
			
			if (deleted_mail)
			{							
				mb->sweep(true);
			}
			
			mail_gc (client.stream,GC_ELT | GC_ENV | GC_TEXTS);
			
		}
		
		else
			Log::info << "Nothing to be done." << Log::endl;
	}
	
	Log::info << "End of message checking." << Log::endl;
	Log::info << "Succesfully completed in " << (time(NULL) - start_time) << " seconds." << Log::endl;  
	
	finalize(client, *maildb);

}

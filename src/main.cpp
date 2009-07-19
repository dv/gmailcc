#include <iostream>

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
	cout << "Gmail Carbon Copy v0.1.x - released July 2009" << endl;
	cout << "http://code.crowdway.com/gmailcc" << endl;
	cout << "Copyright (c) 2009 David Verhasselt (david@crowdway.com)" << endl;
	cout << endl;
	cout << "Compiled at " << __TIMESTAMP__ << " using g++ " << __VERSION__ << endl;
}
	
 
void finalize(Client& client, MailDatabase& maildb, char* error = NULL)
{
	if (error != NULL)
		Log::error << "Finalizing with error: " << error << Log::endl;
	else
		Log::error << "Finalizing without error." << Log::endl;

	client.disconnect();
	
	maildb.save();
	
	cout << "Finalized. Aborting..." << endl;
	
	abort();	
}
 

int main(int argc, char* argv[])
{
	// Check configuration
	Options options(argc, argv);
	
	if (options.get_version()) {
		show_version(); return 1;
	}
	
	if (options.get_help() || options.incomplete())	{
		options.show_help(); return 1;
	}

	// Start logger
	Log::set_priority(options.get_loglevel());
	Log::info << "Started" << Log::endl;
 
 	// Variables
	Client client;
	MailBox* mb;
	MailRecord* mr;
	ENVELOPE* envelope;
	MESSAGECACHE* msgcache;
	
	// Load local database
	MailDatabase* maildb = MailDatabase::load(options.get_maildir_path());

	// Connect to IMAP server
	try {
		client.connect(options.get_username(), options.get_password());
	} catch (AuthClientException &e) {
		Log::critical << "Unable to log-in to server because of invalid credentials. Please check your username and password" << Log::endl;
		finalize(client, *maildb);
	} catch(ClientException &e) {
		Log::critical << "Unable to log-in to server because of an unknown reason. Please check your internet connection." << Log::endl;
		finalize(client, *maildb);
	}
	
	Log::info << "Connection succesfully established. Log-in succesful." << Log::endl;
	
	
	// Load primary mailboxes
	client.open_mailbox("[Gmail]/All Mail");
	mail_fetchfast(client.stream, "1:*");			// We'll need to check the flags either way
	mb = maildb->add_mailbox("[Gmail]/All Mail");
	
	
	bool new_mail;
	bool deleted_mail;
	bool refresh_uids = client.uid_validity != mb->uid_validity;
	//bool incomplete;
	
	unsigned long uid;
	char* body_data; string body;	
	long unsigned int body_length;
	
	mb->uid_validity = client.uid_validity;
	mb->next_uid = client.uid_next;
	mb->messagecount = client.count_messages;
	
	//incomplete = mb->messagecount != mb->mails.size();
	
	Log::info << "Mailbox has " << client.count_messages << " messages." << Log::endl;
	// << client.count_messages << client.get_cachecount() << endl;
	//cout << "Checking " << client.count_messages << " (" << client.get_cachecount() << ") messages." << endl;
	
	for(client.msg_index = 1; client.msg_index <= client.get_cachecount(); client.msg_index++) {
		Log::info << "Message index: " << client.msg_index << ", Count: " << client.get_cachecount() << Log::endl;
		
		mr = NULL;
	
		if (!refresh_uids) {
			mr = maildb->get_mail(mail_uid(client.stream, client.msg_index));		// Try to load the email from our local database using the UID (long)
		}
		
		if (mr == NULL) {	// If UID is invalid or it's a new mail
			envelope = mail_fetchenvelope(client.stream, client.msg_index);			// First fetch the mail metadata
			
			if (envelope == NIL) finalize(client, *maildb, "Envelope is NIL");							// Connection broken
			
			mr = maildb->get_mail(envelope->message_id);							// Again, try to load the email from our local database, this time using the message ID (string)
			
			if (mr == NULL)	// New mail
			{
				uid = mail_uid(client.stream, client.msg_index);
				
				if (uid == 0) finalize(client, *maildb, "(new mail) mail_uid is 0");	// Connection broken
				
				body.clear();
				body_data = mail_fetchbody_full(client.stream, client.msg_index, "", &body_length, FT_PEEK | FT_INTERNAL);
				body.append(body_data, body_length);				// Convert to string using the content_length because of possible
																	// binary data inside (and thus also /0 characters.		
											
				if (!body.size()) finalize(client, *maildb, "Body is empty");
				
				
				mr = maildb->new_mail(envelope->message_id, uid, body);
			}
			else			// Existing mail
			{
				uid = mail_uid(client.stream, client.msg_index);				
				if (uid == 0) finalize(client, *maildb, "mail_uid is 0");	// Connection broken
				
				mr = maildb->new_mail(envelope->message_id, mb, uid);		// Just update the UID
			}
		}
		
		msgcache = mail_elt(client.stream, client.msg_index);
		
		if (msgcache == NIL)  finalize(client, *maildb, "mail_elt returned NIL");	// Connection broken
		
		mr->set_flag_draft(msgcache->draft == 1);
		mr->set_flag_flagged(msgcache->flagged == 1);
		mr->set_flag_passed(msgcache->recent == 1);			// Not sure of this mapping here
		mr->set_flag_replied(msgcache->answered == 1);
		mr->set_flag_seen(msgcache->seen == 1);
		mr->set_flag_trashed(msgcache->deleted == 1);
		
		// Set touched to true to signify this mail shouldn't be deleted
		mr->mark();
	}
	
	// Also check trash & spam here
	
	mail_gc (client.stream,GC_ELT | GC_ENV | GC_TEXTS);
	
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
		deleted_mail = new_mail || (client.count_messages < mb->messagecount);		// If there's new mail, there's no way of knowing presearch if there are any deleted messages.
		refresh_uids = client.uid_validity != mb->uid_validity;
		
		Log::info << "This mailbox has possibly ";
		if (!new_mail) Log::info << "no "; Log::info << "new mail, ";
		if (!deleted_mail) Log::info << "no "; Log::info << "deleted mail, and ";
		if (!refresh_uids) Log::info << "no "; Log::info << "refreshed UID's" << Log::endl; 
		
		mb->uid_validity = client.uid_validity;
		mb->next_uid = client.uid_next;
		mb->messagecount = client.count_messages;
		
		Log::info << "Checking " << client.count_messages << " (" << client.get_cachecount() << ") messages." << Log::endl;
		
		
		if (new_mail || deleted_mail || refresh_uids)
		{		
			for(client.msg_index = 1; client.msg_index <= client.get_cachecount(); client.msg_index++)
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
				// Loop through every maillink to check if it should be deleted
				/*for(vector<MailLink*>::reverse_iterator iter = mb->mails.rbegin(); iter < mb->mails.rend(); )
				{
					if (!(*iter)->touched)	// It's not touched, delete it
					{
						maildb->remove_mail((*iter)->mailrecord, mb);		
					}
					else
						iter++;
				}*/
				
				mb->sweep();
			}
			
			mail_gc (client.stream,GC_ELT | GC_ENV | GC_TEXTS);
			
		}
		
		else
			Log::info << "Nothing to be done." << Log::endl;
	}
	
	Log::info << "End of message checking." << Log::endl;
	
	// Use "mail_elt" for mails we've already got, and we know the UID of
	// Use "fetch_enveloppe" for mails we've already got, but we don't know the UID of (e.g. UID invalid or secondary mailbox)
	// Use "fetch_text" & "fetch_header" for mails we don't have
	/*
	// For all mails in our inventory:
	mail_fetchfast(client.stream, "1:*");	
	MESSAGECACHE *mcache;		
	
	for(client.msg_index = 1; client.msg_index <= client.count_messages; client.msg_index++)
	{
		mcache = mail_elt(client.stream, client.msg_index);
		
		cout << "Mail " << client.msg_index << " (uid: " << mcache->cclientPrivate.uid << ")" << endl;
		cout << " - seen: " << mcache->seen << endl;
		cout << " - answered: " << mcache->answered << endl;
		cout << " - recent : " << mcache->recent << endl;
		cout << endl;
	}
	*/
	
	// For every secondary mailbox:
	/*
	 * 1) msgcount()
	 * 2) if (UID is valid && count == msg_count() && nextUID == nextUID()))
	 * 			NOTHING TO BE DONE
	 * 3) if (nextUID != nextUID())
	 * 		NEW MAIL inside
	 * 		possibly removed
	 * 		possibly other removed, others added
	 * 			-> need to inspect ALL mail
	 * 
	 * 4) if (nextUID == nextUID() && count != msg_count())
	 * 		MAIL deleted
	 * 		need to inspect ALL mail
	 */
	// For all mails not in our inventory:
	/*
	for(client.msg_index = 1; client.msg_index <= client.count_messages; client.msg_index++)
	{
		envelope = mail_fetchenvelope(client.stream, client.msg_index);
		maildb->new_mail(envelope->message_id, mail_uid(client.stream, client.msg_index), mail_fetchheader(client.stream, client.msg_index),mail_fetchtext(client.stream, client.msg_index));
		
		cout << "Mail " << client.msg_index << ": '" << envelope->subject << "' with message-id: " << envelope->message_id << endl;	
	}
	
	// Load secondary mails
	client.open_mailbox("Label2");
	msgcount = client.get_mailcount();
	
	cout << msgcount << endl;

	for(vector<MailBox*>::iterator iter = maildb->mailboxes.begin(); iter < maildb->mailboxes.end(); iter++)
	{
		if ((*iter)->name.compare("[Gmail]/All Mail") != 0)
		{
			client.open_mailbox((*iter)->name);
			msgcount = client.get_mailcount();
			
			cout << "Submailbox " << (*iter)->name << endl;
			
			for(client.msg_index = 1; client.msg_index <= client.count_messages; client.msg_index++)
			{
				envelope = mail_fetchenvelope(client.stream, client.msg_index);
				maildb->new_mail(envelope->message_id, *iter, mail_uid(client.stream, client.msg_index));
			
				cout << "Mail " << client.msg_index << ": '" << envelope->subject << "' with message-id: " << envelope->message_id << endl;	
			}
		}
	}
	
	*/
	
	Log::info << "Ended succesfully" << Log::endl;
	
	finalize(client, *maildb);

}

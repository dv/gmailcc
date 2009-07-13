#include <iostream>

#include "Client.h"
#include "MailDatabase.h"
#include "MailRecord.h"
#include "Log.h"

/*
 * Opened file ./.[Gmail].All Mail/1231624657.P22164Q1913.server.crowdway.com2,
Opened file ./.[Gmail].All Mail/1231624658.P22164Q1914.server.crowdway.com2,
?Bad msgno 1921 in mail_elt, nmsgs = 1920
Aborted
You have new mail in /var/mail/david
david@server:~/GMailBackup/Debug$ ls -lh
 * 
 * 
 * 
 * 
 * TODO: A mail could transfer between All Mail and Trash, two different primary boxes. Our app would remove it from the first and then redownload for the second -> improve?
 * 			-> first check spam & trash

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
 
void finalize(Client& client, MailDatabase& maildb)
{
	cout << "Finalizing" << endl;
	client.disconnect();
	
	maildb.save();
	
	cout << "Finalized" << endl;
	
	abort();	
}
 

int main()
{
	// Create logger
	Log::set_priority(1);
	
	Client client;
	
	// Load local database
	MailDatabase* maildb = MailDatabase::load(string("/home/david/poetry/gmailbackup/maildir/"));
	MailBox* mb;
	MailRecord* mr;
	ENVELOPE* envelope;
	MESSAGECACHE* msgcache;
		
	client.connect("test@crowdway.com", "password");
	//client.connect("david@crowdway.com", "password");
	
	// Load primary mails
	client.open_mailbox("[Gmail]/All Mail");
	mail_fetchfast(client.stream, "1:*");			// We'll need to check the flags either way
	mb = maildb->add_mailbox("[Gmail]/All Mail");
	
	
	bool new_mail;
	bool deleted_mail;
	bool refresh_uids = client.uid_validity != mb->uid_validity;
	//bool incomplete;
	
	unsigned long uid;
	char* header;
	char* content;
	
	mb->uid_validity = client.uid_validity;
	mb->next_uid = client.uid_next;
	mb->messagecount = client.count_messages;
	
	//incomplete = mb->messagecount != mb->mails.size();
	
	
	cout << "Checking " << client.count_messages << " (" << client.get_cachecount() << ") messages." << endl;
	
	for(client.msg_index = 1; client.msg_index <= client.get_cachecount(); client.msg_index++)
	{
		cout << "Message index: " << client.msg_index << ", Count: " << client.get_cachecount() << endl;
		
		mr = NULL;
	
		if (!refresh_uids)
		{
			mr = maildb->get_mail(mail_uid(client.stream, client.msg_index));		// Try to load the email from our local database using the UID (long)
		}
		
		if (mr == NULL)		// If UID is invalid or it's a new mail
		{
			envelope = mail_fetchenvelope(client.stream, client.msg_index);			// First fetch the mail metadata
			
			if (envelope == NIL) finalize(client, *maildb);							// Connection broken
			
			mr = maildb->get_mail(envelope->message_id);							// Again, try to load the email from our local database, this time using the message ID (string)
			
			if (mr == NULL)	// New mail
			{
				uid = mail_uid(client.stream, client.msg_index);
				
				if (uid == 0) finalize(client, *maildb);	// Connection broken
				
				header = mail_fetchheader_full(client.stream, client.msg_index, NULL, NULL, FT_PREFETCHTEXT);
				
				if (!header[0]) finalize(client, *maildb); // Conection broken
				
				content = mail_fetchtext_full(client.stream, client.msg_index, NULL, FT_PEEK);
				
				if (!content[0]) finalize(client, *maildb); // Connection broken					
				
				
				mr = maildb->new_mail(envelope->message_id, uid, header, content);
			}
			else			// Existing mail
			{
				uid = mail_uid(client.stream, client.msg_index);				
				if (uid == 0) finalize(client, *maildb);	// Connection broken
				
				mr = maildb->new_mail(envelope->message_id, mb, uid);		// Just update the UID
			}
		}
		
		msgcache = mail_elt(client.stream, client.msg_index);
		
		if (msgcache == NIL)  finalize(client, *maildb);	// Connection broken
		
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

	cout << "Hier geraakt" << endl;
					
	// Load secondary mails
	client.get_mailboxen();	
	for (vector<string>::iterator it = client.mailboxlist.begin(); it < client.mailboxlist.end(); it++)
	{
		cout << "Checking mailbox " << *it << "..." << endl;
		
		if (it->compare("[Gmail]/All Mail") == 0)
			continue;
			
		if (it->compare("[Gmail]/Trash") == 0)
			continue;
			
		mb = maildb->add_mailbox(*it);	
		client.open_mailbox(mb->name);
		
		new_mail = client.uid_next != mb->next_uid;
		deleted_mail = new_mail || (client.count_messages < mb->messagecount);		// If there's new mail, there's no way of knowing presearch if there are any deleted messages.
		refresh_uids = client.uid_validity != mb->uid_validity;
		
		if (new_mail) cout << "New mail! ";
		if (deleted_mail) cout << "Deleted mail! ";
		if (refresh_uids) cout << "Old UID's!";
		cout << endl;
		
		mb->uid_validity = client.uid_validity;
		mb->next_uid = client.uid_next;
		mb->messagecount = client.count_messages;
		
		cout << "Checking " << client.count_messages << " (" << client.get_cachecount() << ") messages." << endl;
		
		
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
			cout << "Nothing to be done." << endl;
	}
	
	cout << "End of message checking." << endl;
	
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
	
	cout << "Ended succesfully";
	
	finalize(client, *maildb);

}

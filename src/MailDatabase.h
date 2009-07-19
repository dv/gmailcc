#ifndef MAILDATABASE_H_
#define MAILDATABASE_H_

#include <string>
#include <vector>


#include "MailBox.h"
class MailRecord;

/*
 * IMAP Maildir++ Format:
 * ~/Maildir/cur		-> These mails are "seen" or "read"
 * ~/Maildir/tmp		-> new mails are stored here. Upon completion, they are moved to new
 * ~/Maildir/new		-> These mails are not "seen" or "read"
 * ~/Maildir/.subfolder	-> A subfolder
 * ~/Maildir/.subfolder/cur ... 	-> Each folder and subfolder needs cur/tmp/new to be recognized
 * 
 * Possible FAQ: My Labels aren't showing up in my webmail client. Please make sure you are
 * subscribed to these folders. Possibly no way to enforce this using Maildir
 * 
 * Tested on Courier-imap-ssl & roundcube:
 * Hard links work
 * Symbolic links work if absolute to root or userdir (~)
 * 	Example:	ln -s /home/david/maildir/cur/bla ./ works
 * 				ln -s ~/maildir/cur/bla ./ works
 * 				ln -s ../../cur/bla ./ does not work
 * 
 * Oddness:
 * 		When using the info section to mark a message as non-read (as in, removing the S), the "new message" counter
 * 		next to the label is increased, yet when opening the folder the message's read status looks "cached" and set as "read".
 * 
 *		When moving the original to which the link points, the non-read counter next to the label counts the dead-links as well,
 * 		but when opening the label the dead-links mail are not shown.
 * 
 * Maildir++:
 * http://www.courier-mta.org/imap/README.maildirquota.html
 * 
 * Dovecot has some specifics on migration, including UID/UIDVALIDITY integrity, message flags, and
 * mailbox subscription list: http://wiki.dovecot.org/Migration
 * 
 * Courier Specific Maildir Extensions
 * -----------------------------------
 * 
 * ~/Maildir/courierimapsubscribed				A file containing on each line a folder to which the user is subscribed
 * 												Example:	INBOX.Label1
 * 
 * Keywords: http://www.courier-mta.org/imap/README.imapkeywords.html
 * 
 * Dovecot
 * -------
 * 
 * Courier's courierimapsubscribed file is compatible with Dovecot's subscriptions file,
 * but you need to remove the "INBOX." prefixes from the mailboxes. 
 * ~/Maildir/.subscriptions (of ~/mail/.subscriptions)
 * 
 * UW-IMAP
 * -------
 * 
 * Subscription list: ~/.mailboxlist
 * 
 */

class MailDatabase
{
public:
	vector<MailBox*> mailboxes;
	vector<MailRecord*> messages;
	
	static MailDatabase* load(string path);
	static MailDatabase* create(string path);
	void save();

	MailBox* add_mailbox(string mailbox);
	void remove_mailbox(string mailbox);
	
	bool mail_exists(unsigned long uid);
	bool mail_exists(string mailbox, unsigned long uid);	
	
	MailRecord* get_mail(MailBox* mailbox, unsigned long uid);
	MailRecord* get_mail(unsigned long uid);
	
	vector<MailRecord*>::iterator get_imail(string messageid);
	MailRecord* get_mail(string messageid);
	
	MailRecord* add_mail(string messageid, MailBox* mailbox, unsigned long uid, string path);	// Use it when loading database
	
	MailRecord* new_mail(string messageid, MailBox* mailbox, unsigned long uid);	// Add new mail to secondary mailbox, no text necessary
	MailRecord* new_mail(string messageid, unsigned long uid, string body);
	MailRecord* new_mail(string messageid, unsigned long uid, string header, string content);		// Add new mail to All Messages
	
	
	void remove_mail(MailRecord* mr, MailBox* mailbox = NULL);
	void remove_mail(string messageid, MailBox* mailbox = NULL);
	
	void sweep();
	
	string get_maildir();

	MailDatabase();
	virtual ~MailDatabase();
	
private:
	string maildir;
	
	bool is_primary(string mailbox);
	MailBox* get_mailbox(string mailbox);
	
	static void create_maildir(string path);
	
};

#endif /*MAILDATABASE_H_*/

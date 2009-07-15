#ifndef CLIENT_H_
#define CLIENT_H_ 

#include <iostream>
#include <string>
#include <vector>

extern "C" {
	#include "c-client/c-client.h"
}

#undef T
#undef min
#undef max

#define SERVER "{imap.gmail.com:993/ssl}"
#define SERV_INBOX "{imap.gmail.com:993/ssl}INBOX"
#define SERV_ALL "{imap.gmail.com:993/ssl}[Gmail]All Mail"
#define MAIL_LIST_REFERENCE "{imap.gmail.com}"
#define MAIL_LIST_PATTERN "*"

#define MSG_INVALID_CREDENTIALS "[ALERT] Invalid credentials (Failure)"

//struct MAILSTREAM;

using namespace std;

/*
 * Implements an IMAP client with every function we need.
 */

class Client
{
public:
	static Client *active;
	
	unsigned long count_messages;
	unsigned long count_recent;
	unsigned long count_unseen;
	
	unsigned long uid_validity;
	unsigned long uid_next;
	
	unsigned long msg_index;	// Used when looping on msgnumber through a mailbox
								// Gets decremented when a mail is EXPUNGE'd that < msg_index
	
	bool invalid_credentials;
	MAILSTREAM* stream;
	
	vector<string> mailboxlist;	// List with mailboxes, with {...} removed already

	bool connect(string username, string password);
	void disconnect();
	void get_mailboxen();
	void open_mailbox(string mailbox);				// Set the current mailbox. Also calls refresh_mailbox()
	void refresh_mailbox(string mailbox);			// Retrieve the messagecount, UID-next and UID-validity
	unsigned long get_mailcount();
	unsigned long get_mailcount(string mailbox);
	unsigned long get_cachecount();
	
	string remote(string mailbox);

	Client();	
	virtual ~Client();
	
	/* Callback Functions */
	void mm_login (NETMBX *mb,char *user,char *pwd,long trial);
	void mm_list (MAILSTREAM *stream, char delim, char *name, long attrib);
	void mm_status (MAILSTREAM *stream, char *mailbox, MAILSTATUS *status);
	void mm_expunged (MAILSTREAM *stream,unsigned long number);
	
private:
	string username;
	string password;


	//string current_mailbox;
	//void change_mailbox(string new_mailbox);
	
};

#endif /*CLIENT_H_*/

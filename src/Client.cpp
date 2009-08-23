/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#include <exception>

#include "Client.h"
#include "LogSink.h"

Client* Client::active;

void Client::open_stream(string mailbox, long options) throw(ClientException)
{
	invalid_credentials = false;
	web_login_required = false;
	char* mb = cpystr(remote(mailbox).c_str());
	
	//close_mailbox();
	stream = mail_open(stream, mb, options | OP_READONLY);
	
	free(mb);
	
	if (stream == NULL) {
		if (invalid_credentials)
			throw InvalidAuthClientException(this);
		else if (web_login_required)
			throw WebAuthClientException(this);
		else
			throw ClientException(this);
	}	
}

void Client::connect(string username, string password) throw(ClientException)
{
	this->username = username;
	this->password = password;
	
	open_stream(SERV_INBOX);
	
	// Get the mailboxes as initialization
	get_mailboxen();
}

void Client::open_mailbox(string mailbox) throw(ClientException)
{
	open_stream(mailbox);	
	refresh_mailbox(mailbox);	
}

void Client::close_mailbox()
{
	if (stream)
		stream = mail_close(stream);	
}

void Client::refresh_mailbox(string mailbox)
{
	char* mb = cpystr(remote(mailbox).c_str());
	
	mail_status(stream, mb, SA_MESSAGES | SA_UIDNEXT | SA_UIDVALIDITY);
	
	free(mb);	
}

void Client::get_mailboxen()
{
	mailboxlist.clear();
	mail_list(stream, MAIL_LIST_REFERENCE, MAIL_LIST_PATTERN);
}

unsigned long Client::get_mailcount()
{
	return stream->nmsgs;
}
/*
 * These two functions are deprecated since we found out Gimap (Gmail's IMAP server) has a bug
 * where stream->nmsgs is smaller than the count returned by mail_status. stream->nmsgs is the
 * correct number.
 */
 /*
unsigned long Client::get_mailcount()
{
	mail_status(stream, stream->mailbox, SA_MESSAGES);
	
	return count_messages;
	
}

unsigned long Client::get_mailcount(string mailbox)
{
	char* mb = cpystr(remote(mailbox).c_str());
	
	mail_status(stream, mb, SA_MESSAGES);
	
	free(mb);
	
	return count_messages;
}
*/

void Client::disconnect()
{
}

string Client::remote(string mailbox)
{
	if (mailbox[0] != '{')
		return mailbox.insert(0, SERVER);
	else
		return mailbox;
}

Client::Client()
{
	#include "c-client/linkage.c"
	
	Client::active = this;
	stream = NULL;
	
	invalid_credentials = false;
	web_login_required = false;
	fatal_error = false;
	
	/* Set operational parameters */
	mail_parameters(NIL, SET_MAXLOGINTRIALS, (void *) DEF_MAXLOGINTRIALS);
	mail_parameters(NIL, SET_OPENTIMEOUT, (void * ) DEF_OPENTIMEOUT);
}

Client::~Client()
{
}

/* Callback Functions */
void Client::mm_login (NETMBX *mb,char *user,char *pwd,long trial)
{
	Log::info << "[c-client] Trying to log in (take " << trial << ")" << Log::endl; 
		
	strcpy(user, username.c_str());
	strcpy(pwd, password.c_str());	
}

void Client::mm_list(MAILSTREAM *stream, char delim, char *name, long attrib)
{
	string mailbox = name;	
	
	mailboxlist.push_back(mailbox.substr(mailbox.find("}")+1, string::npos));	
}

void Client::mm_status (MAILSTREAM *stream, char *mailbox, MAILSTATUS *status)
{
  //if (status->flags & SA_MESSAGES) this->count_messages = status->messages;
  if (status->flags & SA_RECENT) this->count_recent = status->recent;
  if (status->flags & SA_UNSEEN) this->count_unseen = status->unseen;
  if (status->flags & SA_UIDVALIDITY) this->uid_validity = status->uidvalidity;
  if (status->flags & SA_UIDNEXT) this->uid_next = status->uidnext;
}

void Client::mm_expunged (MAILSTREAM *stream,unsigned long number)
{
	// "number" is removed from the mailbox, so every subsequent message's number has been decremented

	//count_messages--;
	
	// However, this shouldn't affect our FETCH operations since according to the rfc's
	// the server MUST NOT send an EXPUNGE in response to a FETCH, to "maintain sync
	// with client". So the following lines may just be useless.
	if (msg_index > number)
	{
		msg_index--;
	}
}		


/* Interfaces to C-client */


void mm_searched (MAILSTREAM *stream,unsigned long number)
{
}


void mm_exists (MAILSTREAM *stream,unsigned long number)
{
}


void mm_expunged (MAILSTREAM *stream,unsigned long number)
{
	// This function is apparently not called when a mail
	// gets expunged by another client connected at the 
	// same time.
	
	// However, when a mail gets expunged we do get this
	// output:
	// mm_log: %Unknown message data: 1800 EXPUNGE
	// After (!) a call to the server which is not a fetch
	// call:
	
	// 3856 Message index: 1918, Count: 1918
	// 3857 Checking mailbox Business...
	// 3858 mm_log: %Unknown message data: 1800 EXPUNGE
	// 3859 mm_log: %Unknown message data: 1882 EXPUNGE
	// 3860 mm_log: %Unknown message data: 1884 EXPUNGE
	// 3861 mm_log: %Unknown message data: 1886 EXPUNGE
	// 3862 mm_log: %Unknown message data: 1893 EXPUNGE	
	
	printf("Expunged %lu\n", number);
	Client::active->mm_expunged(stream, number);
}


void mm_flags (MAILSTREAM *stream,unsigned long number)
{
	// This function is called repeatedly for every message
	// after a mail_fetchfast() call.
	
	// printf("Flags!\n");
}


void mm_notify (MAILSTREAM *stream,char *string,long errflg)
{
	mm_log (string,errflg);
}


void mm_list (MAILSTREAM *stream,int delimiter,char *mailbox,long attributes)
{
	Client::active->mm_list(stream, delimiter, mailbox, attributes);
}


void mm_lsub (MAILSTREAM *stream,int delimiter,char *mailbox,long attributes)
{
  putchar (' ');
  if (delimiter) putchar (delimiter);
  else fputs ("NIL",stdout);
  putchar (' ');
  fputs (mailbox,stdout);
  if (attributes & LATT_NOINFERIORS) fputs (", no inferiors",stdout);
  if (attributes & LATT_NOSELECT) fputs (", no select",stdout);
  if (attributes & LATT_MARKED) fputs (", marked",stdout);
  if (attributes & LATT_UNMARKED) fputs (", unmarked",stdout);
  putchar ('\n');
}


void mm_status (MAILSTREAM *stream,char *mailbox,MAILSTATUS *status)
{
  Client::active->mm_status(stream, mailbox, status);
}


/* Here come all the error message through. Some samples:
 * Wrong credentials:
 * 	long WARN or PARSE, message: [ALERT] Invalid credentials (Failure)
 * 	No internet connection:
 * 	long ERROR, message: No such host as imap.gmail.com
 * 
 *  Wrong IP/DNS:
 * 	it just waits endlessly after NIL - Trying IP address [xx.xx.xx.xx]
 * 	after some minutes it gives:
 * 		long ERROR, message: Can't connect to xxxxxxxxx: Connection timed out
 */

void mm_log (char *msg,long errflg)
{
	switch ((short) errflg) {
	case NIL:
		Log::info << "[c-client] [warning] " << msg << Log::endl;
		break;
	case PARSE:
	case WARN:
		Log::info << "[c-client] [warning] " << msg << Log::endl;
		
		if (strcmp(msg, MSG_INVALID_CREDENTIALS) == 0)
			Client::active->invalid_credentials = true;
			
		if (strcmp(msg, MSG_WEB_LOGIN_REQUIRED) == 0)
			Client::active->web_login_required = true;
		
		break;
	case ERROR:
		Log::info << "[c-client] [error] " << msg << Log::endl;
		
		if ((strcmp(msg, MSG_CONNECTION_LOST) == 0) ||
			(strcmp(msg, MSG_CONNECTION_BROKEN) == 0) ||
			(strcmp(msg, MSG_SYSTEM_ERROR) == 0))
		{
			Log::error << "[c-client] Fatal error detected" << Log::endl;
			Client::active->fatal_error = true;
		}
				
		break;
	default:
		Log::info << "[c-client] " << msg << Log::endl;
		break; 
	}
}


void mm_dlog (char *string)
{
  Log::info << (string) << Log::endl;
}


void mm_login (NETMBX *mb,char *user,char *pwd,long trial)
{
	Client::active->mm_login(mb, user, pwd, trial);
}


void mm_critical (MAILSTREAM *stream)
{
	printf("mm_critical\n");
}


void mm_nocritical (MAILSTREAM *stream)
{
	printf("mm_nocritical\n");
}


long mm_diskerror (MAILSTREAM *stream,long errcode,long serious)
{
	
#if UNIXLIKE
  kill (getpid (),SIGSTOP);
#else
  abort ();
#endif
  return NIL;
  
}


void mm_fatal (char *string)
{
  printf ("?%s\n",string);
}

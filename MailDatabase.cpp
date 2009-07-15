#include "MailDatabase.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "LogSink.h"
#include "MailLink.h"
#include "MailBox.h"
#include "MailRecord.h"


MailBox* MailDatabase::add_mailbox(string mailbox)
{
	MailBox* box = get_mailbox(mailbox);
	
	if (box != NULL)
		return box;
		
	box = new MailBox(this);
	box->name = mailbox;
	
	if (is_primary(mailbox))
		box->primary = true;
	
	// Doesn't exist yet, create it
	mkdir(box->path().c_str(), 0700);
	mkdir((box->path() + "/cur").c_str(), 0700);
	mkdir((box->path() + "/new").c_str(), 0700);
	mkdir((box->path() + "/tmp").c_str(), 0700);
	
	mailboxes.push_back(box);
	
	return box;
}

bool MailDatabase::mail_exists(unsigned long uid)
{
	return mail_exists("{imap.gmail.com}[Gmail]/All Mail", uid);
	
}

bool MailDatabase::mail_exists(string mailbox, unsigned long uid)
{
	MailBox* mb = get_mailbox(mailbox);
	
	if (mb != NULL)
	{
		for (vector<MailLink*>::iterator iter = mb->mails.begin(); iter < mb->mails.end(); iter++)
		{
				if ((*iter)->uid == uid)
					return true;
		}
	}
	
	return false;
}

/*
 * Find the iterator in the messages list corresponding the mail with given messageid
 */
vector<MailRecord*>::iterator MailDatabase::get_imail(string messageid)
{
	for (vector<MailRecord*>::iterator iter = messages.begin(); iter < messages.end(); iter++)
	{
			//cout << "Comparing " << messageid << " with " << (*iter)->messageid << ": ";
			//cout << messageid.compare((*iter)->messageid) << endl;
			if (messageid.compare((*iter)->messageid) == 0)
			{
				return iter;	
			}
	}
	
	return messages.end();	
}

MailRecord* MailDatabase::get_mail(MailBox* mailbox, unsigned long uid)
{
	for (vector<MailLink*>::iterator iter = mailbox->mails.begin(); iter < mailbox->mails.end(); iter++)
	{
		if ((*iter)->uid == uid)
			return (*iter)->mailrecord;			
	}
	
	return NULL;	
}

MailRecord* MailDatabase::get_mail(unsigned long uid)
{
	return get_mail(get_mailbox("[Gmail]/All Mail"), uid);
}

MailRecord* MailDatabase::get_mail(string messageid)
{
	vector<MailRecord*>::iterator iter = get_imail(messageid);
	
	if (iter == messages.end())	
		return NULL;
	else
		return *iter;	
}

MailRecord* MailDatabase::add_mail(string messageid, MailBox* mailbox, unsigned long uid, string path)
{
	MailRecord* mr = get_mail(messageid);
	
	if (mr == NULL)
	{
		mr = new MailRecord;
		
		mr->messageid = messageid;
		messages.push_back(mr);
	}
	
	MailLink* ml = mr->add_to_mailbox(mailbox, uid, path);
	
	if (mailbox->primary)
	{
		mr->mainlink = ml;
		mr->load_md_info();
	}
	
	return mr;
}

MailRecord* MailDatabase::new_mail(string messageid, MailBox* mailbox, unsigned long uid)
{
	MailRecord* mr = get_mail(messageid);
	
	if (mr == NULL)
		return NULL;
		
	mr->add_to_mailbox(mailbox, uid);
	
	return mr;
}

MailRecord* MailDatabase::new_mail(string messageid, unsigned long uid, string header, string content)
{
	MailRecord* mr = get_mail(messageid);
	
	if (mr != NULL)
		return mr;
	else
		mr = new MailRecord;
	
	mr->messageid = messageid;
	mr->mainlink = mr->add_to_mailbox(get_mailbox("[Gmail]/All Mail"), uid);
	mr->save_content(header, content);
	
	messages.push_back(mr);
	
	return mr;
}

void MailDatabase::remove_mail(MailRecord* mr, MailBox* mailbox)
{		
	if (mr == NULL)
		return;
		
	if (mailbox != NULL)
		mr->remove_from_mailbox(mailbox);
	else
		mr->remove();
	
	if (mr->links.size() <= 0)
	{
		cout << "Searching imail with messageid " << mr->messageid << endl;
		vector<MailRecord*>::iterator mr_iter = get_imail(mr->messageid);
		
		if (mr_iter < messages.end())
		{
			cout << "Erasing imail from messages" << endl;
			messages.erase(mr_iter);
			cout << "Done" << endl;
		}
		else
			cout << "Whoops, something went wrong here" << endl;
	}	
}


void MailDatabase::remove_mail(string messageid, MailBox* mailbox)
{
	remove_mail(get_mail(messageid), mailbox);
}


MailDatabase* MailDatabase::create(string path)
{
}


bool MailDatabase::is_primary(string mailbox)
{
	return mailbox.compare("[Gmail]/All Mail") == 0;	
}

MailBox* MailDatabase::get_mailbox(string mailbox)
{
	for (vector<MailBox*>::iterator it = mailboxes.begin(); it < mailboxes.end(); it++)
	{
		if ((*it)->name.compare(mailbox) == 0)
		{
			return *it;
		}
	}
	
	return NULL;
	
}

/**
 * mailbox-name
 * mailbox-uid_validity
 * mailbox-next_uid
 * mailbox-msgcount
 * maillink
 * maillink
 * maillink
 * /n
 */

void MailDatabase::save()
{
	
	cout << "Saving file ..." << endl;
	
	ofstream dbfile;
	
	dbfile.open("database");
	
	cout << "Is it opened?" << endl;
	
	if (dbfile.is_open())
	{
		cout << "Yes!" << endl;
		
		for (vector<MailBox*>::iterator mb = mailboxes.begin(); mb < mailboxes.end(); mb++)
		{
			//cout << "Mailbox... ";
			
			dbfile << (*mb)->name << endl;		// mailbox-name
			dbfile << (*mb)->uid_validity << endl;			// mailbox-uid_validity
			dbfile << (*mb)->next_uid << endl;			// mailbox-next_uid
			dbfile << (*mb)->messagecount << endl;			// mailbox-msgcount
			
			for (vector<MailLink*>::iterator mail = (*mb)->mails.begin(); mail < (*mb)->mails.end(); mail++)
			{
				//cout << "Mail...";
				dbfile << (*mail)->uid << endl;						// uid
				dbfile << (*mail)->mailrecord->messageid << endl;	// messageid
				dbfile << (*mail)->path << endl;					// path				
			}
			
			//cout << endl;
			
			dbfile << endl;
		}
		
		dbfile.close();		

	}	
}

MailDatabase* MailDatabase::load(string path)
{
	Log::info << "Loading MailDatabase @ " << path << Log::endl;
	
	// Checking if path is correct
	struct stat info;
	if (stat(path.c_str(), &info) == 0)	// File or directory exists
	{
		if (info.st_mode & S_IFREG)
		{
			Log::critical << "Unable to load the database: given path refers to a file, not a directory." << Log::endl;
			return NULL;
		}
		if (! (info.st_mode & S_IWRITE))
		{
			Log::critical << "Unable to load the database: no write permissions to given path." << Log::endl;
			return NULL;
		}
		if (! (info.st_mode & S_IREAD))
		{
			Log::critical << "Unable to load the database: no read permissions to given path." << Log::endl;
			return NULL;
		}
		
		Log::info << "Given path is OK." << Log::endl;
	}
	else
	{
		Log::info << "Given path does not exist or we do not have enough permissions in the parent directory." << Log::endl;
		Log::info << "Trying to create the directory." << Log::endl;
		
		if(mkdir(path.c_str(), 0700) == 0)
		{
			Log::info << "Succesfully created the directory." << Log::endl;
		}
		else
		{
			Log::critical << "Given path does not exist and we were not able to create it." << Log::endl;
			Log::critical << "Please check for existing files and their permissions." << Log::endl;
			
			return NULL;
		}
	}

	MailDatabase* mdb = new MailDatabase();
	
	// Change the working directory
	chdir(path.c_str());
	
	// Load databasefile
	ifstream dbfile;
	
	Log::info << "Opening database-file" << Log::endl;
	dbfile.open("database");
	
	if (dbfile.is_open())
	{
		string line;
		
		MailBox* currentmb;
		
		getline(dbfile, line);
		while(!dbfile.eof())
		{
			currentmb = mdb->add_mailbox(line);
			getline(dbfile, line);	currentmb->uid_validity = atoi(line.c_str());
			getline(dbfile, line);	currentmb->next_uid = atoi(line.c_str());
			getline(dbfile, line);	currentmb->messagecount = atoi(line.c_str());
			
			unsigned long uid;
			string path;
			string messageid;
			
			while(!dbfile.eof())
			{
				getline(dbfile, line);

				if (line.empty())
				{
					break;	// Next mailbox
				}
				
				uid = atoi(line.c_str());
				getline(dbfile, messageid);
				getline(dbfile, path);
				
				mdb->add_mail(messageid, currentmb, uid, path);
			}
			
			getline(dbfile, line);
		}
		
		dbfile.close();
	}
	
	else
	{
		Log::error << "Error while trying to open database file ( " << path <<  "/" << "database" << " )" << Log::endl;
	}
	
	// Load mailboxese
	
	// Return
	return mdb;	
}

void MailDatabase::sweep()
{
	for (vector<MailRecord*>::iterator iter = messages.begin(); iter < messages.end(); )
	{
		if (!(*iter)->marked)
		{
			(*iter)->remove();
			delete *iter;
			
			iter = messages.erase(iter);
		}
		else
			iter++;
	}
	
	for (vector<MailBox*>::iterator iter = mailboxes.begin(); iter < mailboxes.end(); iter++)
	{
		(*iter)->sweep();	
	}
}
	


MailDatabase::MailDatabase()
{
}

MailDatabase::~MailDatabase()
{
	// TODO: free mailboxes
}

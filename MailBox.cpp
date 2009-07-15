#include "MailBox.h"
#include "MailRecord.h"
#include "MailDatabase.h"
#include "MailLink.h"


string MailBox::get_path()
{
	return path;
}

void MailBox::mark()
{
	marked = true;
}

void MailBox::dirty()
{
	dirtied = true;
}

/*
 * Remove mails in this box marked as removed (i.e. touched == false or stale == true)
 */
int MailBox::sweep()
{
	if (!dirtied)
		return 0;
		
	int count = 0;
	
	for(vector<MailLink*>::iterator iter = mails.begin(); iter < mails.end(); )
	{
			if (!(*iter)->staled)	// Stale, this is a removed link
			{
				delete *iter;
				iter = mails.erase(iter);
				count++;
			}
			
			else
				iter++;
	}
	
	messagecount = mails.size();
	cout << "Removed " << count << " messages." << endl;
	
	return count;
}

MailBox::MailBox(MailDatabase* maildb, string name): name(name), maildb(maildb)
{
	path = "." + name; 
	
	// Generate directorypath
	size_t index = 0;	
	while((index = path.find("/", index)) != string::npos)
		path.replace(index, 1, ".");
	
	path = maildb->get_maildir() + path + "/";
}

MailBox::~MailBox()
{
	
}
#include "MailRecord.h"

#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include "LogSink.h"
#include "MailLink.h"
#include "MailBox.h"

int MailRecord::number_of_deliveries = 0;

MailRecord::MailRecord()
{
	flag_draft = false;
	flag_flagged = false;
	flag_passed = false;
	flag_replied = false;
	flag_seen = false;
	flag_trashed = false;
}

MailRecord::~MailRecord()
{
	for (vector<MailLink*>::iterator iter = this->links.begin(); iter < links.end(); iter++)
	{
		delete(*iter);
	}	
}

void MailRecord::mark()
{
	marked = true;
}

bool MailRecord::get_flag_draft()
{
	return flag_draft;
}

void MailRecord::set_flag_draft(bool flag_draft)
{
	if (this->flag_draft == flag_draft)
		return;
		
	this->flag_draft = flag_draft;
	this->flags_changed();
}

bool MailRecord::get_flag_flagged()
{
	return flag_flagged;
}

void MailRecord::set_flag_flagged(bool flag_flagged)
{
	if (this->flag_flagged == flag_flagged)
		return;
		
	this->flag_flagged = flag_flagged;
	this->flags_changed();
}

bool MailRecord::get_flag_passed()
{
	return flag_passed;
}

void MailRecord::set_flag_passed(bool flag_passed)
{
	if (this->flag_passed == flag_passed)
		return;
		
	this->flag_passed = flag_passed;
	this->flags_changed();
}

bool MailRecord::get_flag_replied()
{
	return flag_replied;
}

void MailRecord::set_flag_replied(bool flag_replied)
{
	if (this->flag_replied == flag_replied)
		return;
		
	this->flag_replied = flag_replied;
	this->flags_changed();
}

bool MailRecord::get_flag_seen()
{
	return flag_seen;
}

void MailRecord::set_flag_seen(bool flag_seen)
{
	if (this->flag_seen == flag_seen)
		return;
		
	this->flag_seen = flag_seen;
	this->flags_changed();
}

bool MailRecord::get_flag_trashed()
{
	return flag_trashed;
}

void MailRecord::set_flag_trashed(bool flag_trashed)
{
	if (this->flag_trashed == flag_trashed)
		return;
		
	this->flag_trashed = flag_trashed;
	this->flags_changed();
}

void MailRecord::save_content(string header, string content)
{
	ofstream mfile;	

	mfile.open(mainlink->path.c_str());
	
	if (mfile.is_open())
	{
		mfile << header << endl;
		mfile << content;
		mfile.close();
	}
	else Log::error << "Unable to open file " << mainlink->path << Log::endl;
}

MailLink* MailRecord::add_to_mailbox(MailBox* mailbox, unsigned long uid, string path)
{
	MailLink* maillink = find_link(mailbox);
	
	if (maillink != NULL)
	{
		maillink->uid = uid;
		
		if (!path.empty())
		{
			if (!rename(maillink->path.c_str(), path.c_str()))
				perror("MailRecord::add_to_mailbox: unable to move link");
			else
				maillink->path = path;
		}		
	}
	else
	{		
		maillink = new MailLink;
		
		if (mailbox->primary)
			mainlink = maillink;
			
		bool exists = false;
		if (path.empty())
			path = mailbox->get_path() + "cur/" + generate_md_filename() + get_md_info();
		else
			exists = true;
			
			
		
		maillink->mailrecord = this;
		maillink->mailbox = mailbox;
		maillink->path = path;
		maillink->uid = uid;
		
		this->links.push_back(maillink);
		mailbox->mails.push_back(maillink);
		
		if (!exists && (this->links.size() > 1))	// Create a link to the original
		{
			string sourcepath = this->mainlink->path.c_str();
			//sourcepath.insert(0, "../");
			
			bf::create_hard_link(sourcepath, path);
			//	perror("MailRecord::add_to_mailbox: creating symlink failed");
		}
		
		if (exists)
		{
			load_md_info();
		}
	}
			
	
	return maillink;
}

void MailRecord::remove_from_mailbox(MailBox* mailbox)
{
	vector<MailLink*>::iterator iter = find_ilink(mailbox);
	
	if (iter < this->links.end())
	{
		remove_from_mailbox(iter);
		return;		
	}
	else
	{
		cout << "Whoops, something went wrong here" << endl;
	}
}

vector<MailLink*>::iterator MailRecord::remove_from_mailbox(vector<MailLink*>::iterator iter)
{
	if (unlink((*iter)->path.c_str()) != 0)
			perror("MailRecord::remove_from_mailbox: unable to unlink file");
			
	(*iter)->mailbox->dirty();
	(*iter)->stale();
	
	return this->links.erase(iter);
}
	

void MailRecord::remove()
{
	for (vector<MailLink*>::iterator iter = this->links.begin(); iter < this->links.end(); )
	{
		iter = this->remove_from_mailbox(iter);
	}
}

MailLink* MailRecord::find_link(MailBox* mailbox)
{
	vector<MailLink*>::iterator result = find_ilink(mailbox);
	
	if (result == this->links.end())
		return NULL;
	else
		return *result;
}

vector<MailLink*>::iterator MailRecord::find_ilink(MailBox* mailbox)
{
	for (vector<MailLink*>::iterator iter = this->links.begin(); iter < this->links.end(); iter++)
	{
		if ((*iter)->mailbox == mailbox)
		{
			return iter;
		}
	}
	
	return this->links.end();
}



/**
 * Regenerates the paths of every mail, moves the files
 * of the mails.
 */
void MailRecord::flags_changed()
{	
	for (vector<MailLink*>::iterator iter = this->links.begin(); iter < links.end(); iter++)
	{
		string new_path = (*iter)->path;
		extract_base_path(new_path);
		
		new_path.insert(new_path.size(), this->get_md_info());
		
		if (rename((*iter)->path.c_str(), new_path.c_str()) != 0)
			perror("MailRecord::flags_changed: Unable to rename file");
		else
			(*iter)->path = new_path;				
	}
}

/**
 * Parse the given path and extract the path without the
 * tailing info-part.
 */
void MailRecord::extract_base_path(string& path)
{
	size_t index;
	index = path.rfind(MD_TAIL);
	
	if (index != string::npos)
		path.replace(index, path.length(), "");
	else
		perror("MailRecord::update_path: Unable to parse pathname"); 
}


string MailRecord::generate_md_filename()
{
	char seconds[20];	sprintf(seconds, "%d", time(NULL));

	char hostname[99] = "hostnamex";
	string shostname;
	pid_t pid = getpid();
	
	if (pid < 0)
		perror("MailRecord::generate_md_filename: unable to get pid");
	
	char spid[20];	sprintf(spid, "%d", pid);
	 
	if (gethostname(hostname, 99) != 0)
	{
		perror("MailRecord::generate_md_filename: unable to gethostname()");
		perror(hostname);
	}
	
 	shostname = hostname;
	
	// Replace "/" with "\057"
	size_t index;
	while((index = shostname.find("/")) != string::npos)
	{
			shostname.replace(index, 1, "\057");
	}

	// Replace ":" with "\072"
	while((index = shostname.find(":")) != string::npos)
	{
			shostname.replace(index, 1, "\072");
	}
	
	char delnum[20];	sprintf(delnum, "%d", MailRecord::number_of_deliveries++);
	
	string result;
	
	result += seconds;
	result += ".";
	result += "P";
	result += spid;
	result += "Q";
	result += delnum;
	result += ".";
	result += shostname;
	
	return result;	
}

/**
 * Generate the "info" part of the maildir specification
 * see: http://cr.yp.to/proto/maildir.html
 */
string MailRecord::get_md_info()
{
	string result = MD_TAIL;
	
	if (this->flag_draft) result += "D";
	if (this->flag_flagged) result += "F";
	if (this->flag_passed) result += "P";
	if (this->flag_replied) result += "R";
	if (this->flag_seen) result += "S";
	if (this->flag_trashed) result += "T";
	
	return result;
}

/**
 * Extract the "info" part from the path and update lfags
 */
void MailRecord::load_md_info()
{
	this->flag_draft = false;
	this->flag_flagged = false;
	this->flag_passed = false;
	this->flag_replied = false;
	this->flag_seen = false;
	this->flag_trashed = false;
	
	string path = mainlink->path;	
	for(size_t index = path.rfind(MD_TAIL) + strlen(MD_TAIL); index < path.size(); index++)
	{
				switch(path[index])
				{
					case 'D':
						this->flag_draft = true; break;
					case 'F': break;
						this->flag_flagged = true; break;
					case 'P': break;
						this->flag_passed = true; break;
					case 'R': break;
						this->flag_replied = true; break;
					case 'S': break;
						this->flag_seen = true; break;
					case 'T': break;
						this->flag_trashed = true; break;
				}				
	}
}
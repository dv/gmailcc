#ifndef MAILBOX_H_
#define MAILBOX_H_

#include <string>
#include <vector>

class MailLink;
class MailDatabase;

using namespace std;

class MailBox
{
public:
	static bool is_primary(const string);
	static bool is_inbox(const string);

	bool inbox;
	bool primary;
	unsigned long next_uid;
	unsigned long uid_validity;
	unsigned long messagecount;
	string name;
	vector<MailLink*> mails;

	bool marked;						// Marked for removal
	
	string get_path();
	void mark();
	void dirty();
	int sweep(bool check_marked = false);
	
	MailBox(MailDatabase* maildb, string name);
	~MailBox();
	
private:
	static const char* primaries[];

	string path;
	bool dirtied;						// Needs sweeping to get rid of removed links
	MailDatabase* maildb;
};

#endif /*MAILBOX_H_*/

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
	bool primary;
	unsigned long next_uid;
	unsigned long uid_validity;
	unsigned long messagecount;
	string name;
	vector<MailLink*> mails;

	bool marked;						// Marked for removal
	
	string path();
	void mark();
	void dirty();
	int sweep();
	
	MailBox(MailDatabase* maildb);
	~MailBox();
	
private:
	bool dirtied;						// Needs sweeping to get rid of removed links
	MailDatabase* maildb;
};

#endif /*MAILBOX_H_*/

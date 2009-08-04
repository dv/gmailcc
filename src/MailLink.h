/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#ifndef MAILLINK_H_
#define MAILLINK_H_

#include <string>

class MailRecord;
class MailBox;

using namespace std;

class MailLink
{
public:
	MailRecord* mailrecord;
	MailBox* mailbox;
	unsigned long uid;
	string path;
	
	bool staled;				// This links is stale. It should be removed.
	bool marked;				// Set to true when this link is found on the
								// server. After everything is checked, all
								// non-touched links are regarded as "deleted"
								// and are removed.

	void stale();
	void mark();

	MailLink();
	MailLink(const string box, const unsigned long uid, const string path);
	virtual ~MailLink();
};

#endif /*MAILLINK_H_*/

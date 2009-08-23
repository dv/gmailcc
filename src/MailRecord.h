/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#ifndef MAILRECORD_H_
#define MAILRECORD_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include <iostream>
#undef min
#undef max
#include <fstream>



#define MD_TAIL ":2,"

#include "MailLink.h"
class MailBox;

using namespace std;

/* 
 * A mailrecord is the primary instance of an email. Per email there is only
 * one mailrecord, but for every Label an email has, there is one maillink.
 */

class MailRecord
{
public:
	string messageid;
	vector<MailLink*> links;
	MailLink* mainlink;
	bool marked;
	void mark();
	MailLink* find_link(MailBox* mailbox);		// Get the MailLink corresponding to the give mailbox and this email

	MailRecord();
	virtual ~MailRecord();
	
	// Getters & Setters
	bool get_flag_draft();
	void set_flag_draft(bool flag_draft);
	bool get_flag_flagged();
	void set_flag_flagged(bool flag_flagged);
	bool get_flag_passed();
	void set_flag_passed(bool flag_passed);
	bool get_flag_replied();
	void set_flag_replied(bool flag_replied);
	bool get_flag_seen();
	void set_flag_seen(bool flag_seen);
	bool get_flag_trashed();
	void set_flag_trashed(bool flag_trashed);
	
	void sync_flags();
	
	// Management
	void save_content(string body);
	MailLink* add_to_mailbox(MailBox* mailbox, unsigned long uid, string path = "");
	void remove_from_mailbox(MailBox* mailbox);
	vector<MailLink*>::iterator remove_from_mailbox(vector<MailLink*>::iterator iter);
	void remove();
	void load_md_info(); 
	

	
	
private:
	static int number_of_deliveries;

	bool flag_draft;
	bool flag_flagged;	
	bool flag_passed;
	bool flag_replied;
	bool flag_seen;
	bool flag_trashed;	
		
	void extract_base_path(string& path);
	string get_md_info();

	string convert_to_path(string& mailbox);
	string generate_md_filename();
	vector<MailLink*>::iterator find_ilink(MailBox* mailbox);
	
};

#endif /*MAILRECORD_H_*/

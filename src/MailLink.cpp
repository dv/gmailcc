/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */
 
#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include "LogSink.h"
#include "MailLink.h"
#include "MailRecord.h"
#include "MailBox.h"

#define MD_TAIL ":2,"

MailLink::MailLink()
{
	marked = false;
	staled = false;
}

MailLink::~MailLink()
{
}

void MailLink::mark()
{
	marked = true;
}

void MailLink::stale()
{
	staled = true;
}

string MailLink::get_base_path()
{
	size_t index;
	string result = this->path;
	index = result.rfind(MD_TAIL);
	
	if (index != string::npos)
		result.replace(index, result.length(), "");
	else
		Log::error << "get_base_path(): Unable to parse pathname" << Log::endl;
		
	return result;
}

string MailLink::get_path()
{
	if (bf::exists(this->path))
		return this->path;
	else
	{
		string base = this->get_base_path();
		bf::path p(base);
		
		bf::directory_iterator end_itr;

		for (bf::directory_iterator itr(p.branch_path()); itr != end_itr; ++itr)	// Using branch_path for boost 1.35 compatibility. Use parent_path for 1.39+
		{
			if (itr->path().leaf().find(p.leaf()) != string::npos) 	// Using leaf() for boost 1.35 compatibility. Use filename() for 1.39+
			{
					this->set_path(itr->path().string());				
					return this->path;
			}			
		}		
		
	}
	
	Log::error << "Could not find new filename of " << this->path << " in " << this->mailbox->name << Log::endl;	
	return this->path;
}

void MailLink::set_path(string path)
{	
	this->path = path;	
}
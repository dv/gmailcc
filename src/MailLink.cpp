/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#include "MailLink.h"
#include "MailRecord.h"
#include "MailBox.h"

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

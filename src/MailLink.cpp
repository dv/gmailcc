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

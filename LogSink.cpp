#include "LogSink.h"

LogSink::LogSink()
{
}

LogSink::~LogSink()
{
}


void LogSink::set(Log& log, int priority)
{
	this->log = &log;
	this->priority = priority;	
}
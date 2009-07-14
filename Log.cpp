#include "Log.h"

#include "LogSink.h"

char* Log::endl = "\n";
char* Log::priorities[] = { "debug", "info", "warning", "error", "critical" };


LogSink Log::debug;
LogSink Log::info;
LogSink Log::warning;
LogSink Log::error;
LogSink Log::critical;


Log::Log()
{
	logfile = NULL;
	output = &std::cout;
	priority = priority_warning;
	
	debug.set(*this, priority_debug);
	info.set(*this, priority_info);
	warning.set(*this, priority_warning);
	error.set(*this, priority_error);
	critical.set(*this, priority_critical);	
}

Log::~Log()
{
	if (logfile != NULL)
	{
		output = &std::cout;
		logfile->close();
		delete logfile;
	}
}


void Log::set_logfile(char* path)
{
	Log::getInstance()->logfile = new std::ofstream(path);	
	Log::getInstance()->output = Log::getInstance()->logfile;	
}

void Log::set_priority(int priority)
{
	Log::getInstance()->priority = priority;
}


void Log::new_line()
{
	if (!end_line)
		*output << std::endl;
	else
		end_line = false;
		
	*output << "[" << priorities[current_priority] << "] ";	
}

void Log::begin_entry(int new_priority)
{
	if (priority <= current_priority)
	{		
		if (end_line || (current_priority != new_priority))			// If the priority has changed, assume a new line
		{
			new_line();
		}
	}
}

Log* Log::getInstance()
{
	static Log instance;
	return &instance;
}


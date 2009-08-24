#include "Log.h"

#include "LogSink.h"

char Log::endl = '\n';
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
	line_ended = true;
	
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


void Log::set_logfile(const char* path)
{
	Log::getInstance()->logfile = new std::ofstream(path);	
	Log::getInstance()->output = Log::getInstance()->logfile;	
}

void Log::set_priority(int priority)
{
	Log::getInstance()->priority = priority;
}

bool Log::has_priority(int priority)
{
	return this->priority <= priority;	
}


void Log::end_line()
{
	line_ended = true;
}

void Log::new_line()
{
	if (!line_ended)
		*output << std::endl;
	else
		line_ended = false;
		
	*output << "[" << priorities[current_priority] << "] ";	
}

void Log::begin_entry(int new_priority)
{
	int last_priority = current_priority;
	current_priority = new_priority;
	
	if (!has_priority(new_priority))
		return;

	if (line_ended || (last_priority != new_priority))			// If the priority has changed, assume a new line
		new_line();
}


template<>
Log& Log::operator<<(char const& value)
{
	if (!has_priority(current_priority))
		return *this;
		
	*this->output << value;
	
	if (value == Log::endl)
	{
		(*this->output).flush();
		this->end_line();
	}
	
	return *this;	
}	

Log* Log::getInstance()
{
	static Log instance;
	return &instance;
}


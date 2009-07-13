#include "Log.h"


void Log::debug(char* message)
{
	Log::getInstance()->entry(priority_debug, message);
}

void Log::info(char* message)
{
	Log::getInstance()->entry(priority_info, message);	
}

void Log::warning(char* message)
{
	Log::getInstance()->entry(priority_warning, message);	
}

void Log::error(char* message)
{
	Log::getInstance()->entry(priority_error, message);
}

void Log::critical(char* message)
{
	Log::getInstance()->entry(priority_critical, message);	
}

void Log::set_logfile(char* path)
{
	Log::getInstance()->logfile = new std::ofstream(path);	
}

void Log::set_priority(int priority)
{
	Log::getInstance()->priority = priority;
}

void Log::entry(const int priority, char* message)
{
	if (this->priority <= priority)
	{
		if (logfile)
				*logfile << message << std::endl;
		
		else
			std::cout << message << std::endl;
	}			
}

Log* Log::getInstance()
{
	static Log instance;
	return &instance;
}

Log::Log()
{
	logfile = 0;
	priority = priority_warning;
}

Log::~Log()
{
}

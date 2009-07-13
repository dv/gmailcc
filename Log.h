#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <fstream>

class Log
{
public:
	static const int priority_debug = 1;
	static const int priority_info = 2;
	static const int priority_warning = 3;
	static const int priority_error = 4;
	static const int priority_critical = 5;
	
	static void debug(char* message);
	static void info(char* message);
	static void warning(char* message);
	static void error(char* message);
	static void critical(char* message);
	
	static void set_logfile(char* path);
	static void set_priority(int priority);
	
	
protected:
	Log();
	virtual ~Log();
	
private:	
	int priority;
	std::ofstream* logfile;
	
	static Log* getInstance();	

	void entry(const int priority, char* message);
};

#endif /*LOGGER_H_*/

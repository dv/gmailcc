#ifndef LOG_H_
#define LOG_H_

#include <ostream>
#include <iostream>
#include <fstream>
#include <string>
#include <typeinfo>

/**
 * Simple Log-class 2009 david@crowdway.com
 * 
 * Usage:
 * Log::error << "There be dragons @ " << index << Log::endl;
 * 
 * For more advanced (manipulatable) logger:
 * http://stackoverflow.com/questions/290632/how-to-overload-operator-that-doesnt-take-or-return-ostream
 */

class LogSink;

class Log
{
public:
	static const int priority_debug = 0;
	static const int priority_info = 1;
	static const int priority_warning = 2;
	static const int priority_error = 3;
	static const int priority_critical = 4;
	
	static char* endl;	
	static char* priorities[];
	
	static void set_logfile(char* path);
	static void set_priority(int priority);
	
	static LogSink debug;
	static LogSink info;
	static LogSink warning;
	static LogSink error;
	static LogSink critical;	
	static Log* getInstance();	
	
protected:
	Log();
	virtual ~Log();
	
private:
	int priority;
	std::ofstream* logfile;
	std::ostream* output;
	int current_priority;
	bool end_line;
	
	void new_line();
	void begin_entry(int new_priority);
	
	template <typename S>
	friend LogSink& operator<< (LogSink& sink, S const& value);
	
	template<typename L>
	friend Log& operator<<(Log& log, L const& value);
	
	//static Log* getInstance();	
};


template<typename L>
Log& operator<<(Log& log, L const& value)
{
	*log.output << value;
	
	/*if (typeid(value) == typeid(Log::endl) && (string(value).compare(Log::endl))
	{
		log.end_line = true;
	}*/
	
	return log;
	
}


#endif /*LOGGER_H_*/

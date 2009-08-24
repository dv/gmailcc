#ifndef LOG_H_
#define LOG_H_

#include <ostream>
#include <iostream>
#include <fstream>
#include <string>

/**
 * Simple Log-class 2009 david@crowdway.com
 * 
 * Usage:
 * Log::error << "There be dragons @ " << index << Log::endl;
 * 
 * For more advanced (manipulatable) logger:
 * http://stackoverflow.com/questions/290632/how-to-overload-operator-that-doesnt-take-or-return-ostream
 * http://stdcxx.apache.org/doc/stdlibug/33-2.html
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
	
	static char endl;	
	static char* priorities[];
	
	static void set_logfile(const char* path);
	static void set_priority(int priority);
	
	static LogSink debug;
	static LogSink info;
	static LogSink warning;
	static LogSink error;
	static LogSink critical;	
	static Log* getInstance();
	
			
	template<typename L>
	Log& operator<<(L const& value)
	{
		if (!has_priority(current_priority))
		return *this;
		
		*this->output << value;
	
		return *this;
	}
	
protected:
	Log();
	virtual ~Log();
	
private:
	int priority;
	std::ofstream* logfile;
	std::ostream* output;
	int current_priority;
	void end_line();
	bool line_ended;
	
	bool has_priority(int priority);
	
	void new_line();
	void begin_entry(int new_priority);

	
	friend class LogSink;
	
	//static Log* getInstance();	
};


#endif /*LOGGER_H_*/

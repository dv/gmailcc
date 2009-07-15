#ifndef LOGSINK_H_
#define LOGSINK_H_

#include "Log.h"

class LogSink
{
public:
	LogSink();
	void set(Log& log, int priority);
	virtual ~LogSink();
    
    template<typename S> 
    LogSink& operator<<(S const& value) { 
		(*this->log).begin_entry(this->priority);
		(*this->log) << value;
		
		return *this;
    }

private:
	Log* log;
	int priority;
};

#endif /*LOGSINK_H_*/

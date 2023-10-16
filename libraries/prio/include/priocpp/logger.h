#ifndef MOL_PROMISE_LIBEVENT_PipedProcess_LOGGER_DEF_GUARD_DEFINE_
#define MOL_PROMISE_LIBEVENT_PipedProcess_LOGGER_DEF_GUARD_DEFINE_

/**
 * \file logger.h
 */

#ifndef _WIN32

#include "priocpp/pipe.h"

namespace prio      {



#define LOG_LEVEL_FATAL  0
#define LOG_LEVEL_ERROR  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG  4

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 2
#endif


#if DEBUG_LEVEL > 3
#define LOG_DEBUG(...)  LOG_LEVEL_DEBUG, __func__, __LINE__, __FILE__, __VA_ARGS__
#else
#define LOG_DEBUG( ...) 
#endif

#if DEBUG_LEVEL > 2
#define LOG_INFO( ...) LOG_LEVEL_INFO, __func__, __LINE__, __FILE__, __VA_ARGS__
#else
#define LOG_INFO( ...) 
#endif

#if DEBUG_LEVEL > 1
#define LOG_WARN( ...)  LOG_LEVEL_WARN, __func__, __LINE__, __FILE__, __VA_ARGS__
#else
#define LOG_WARN(...) 
#endif

#if DEBUG_LEVEL > 0
#define LOG_ERROR( ...)  LOG_LEVEL_ERROR, __func__, __LINE__, __FILE__, __VA_ARGS__
#else
#define LOG_ERROR( ...) 
#endif

#define LOG_FATAL(...)  LOG_LEVEL_FATAL, __func__, __LINE__, __FILE__, __VA_ARGS__

class LogConfig
{
public:

	std::vector<std::function<void(std::string)>> appender;
};

inline std::string now()
{
        std::time_t now = std::time(nullptr);

    std::tm* t = std::gmtime(&now);

    std::stringstream oss; 
//              std::locale mylocale("de_DE.utf8");  
//          oss.imbue(mylocale);

    oss << std::put_time(t, "%Y-%m-%dT%H:%M:%S");

    return oss.str();
}


inline void log_printf(std::ostream& oss, const char* s )
{
    while(*s)
    {
        if(*s == '"')
        {
            oss << "\\\"";
            s++;
            continue;
        }

        if(*s == '\n')
        {
            oss << "\\n";
            s++;
            continue;
        }

        if(*s=='%' && *++s != '%')
        {
            throw std::runtime_error("invalid format: missing args");
        }
        oss << *s++;
    }
}

template<class T, class ... Args>
void log_printf(std::ostream& oss, const char* s, T t, Args ... args)
{
    while(*s)
    {
        if(*s == '"')
        {
            oss << "\\\"";
            s++;
            continue;
        }

        if(*s == '\n')
        {
            oss << "\\n";
            s++;
            continue;
        }

        if(*s=='%' && *++s != '%')
        {
            oss << t;
            return log_printf(oss,s,args...);
        }
        oss << *s++;
    }
    throw std::runtime_error("invalid format: extra args");
}


class Logging
{
public:

	Logging(std::shared_ptr<LogConfig> c)
		: pipe_(Pipe::create()), config_(c)
	{
		pipe_->asyncLineReader(config_->appender);
	}

	~Logging()
	{
		pipe_->close();
	}

	void log(std::string s)
	{
		pipe_->write(s)
		.then([](){});
	}

private:

	std::shared_ptr<Pipe> pipe_;
	std::shared_ptr<LogConfig> config_;
};


class Logger
{
public:

	Logger(std::shared_ptr<Logging> l)
		: logger_(l)
	{}

	Logger(const std::string& name, std::shared_ptr<Logging> l)
		: name_(name), logger_(l)
	{
	}

	~Logger()
	{}

	template<class ... Args>
	void log(int sev , const char* func, int line, const char* file, const char* msg, Args ... args)
	{
		std::ostringstream oss;
        oss << "{\"severity\":\"";
		switch(sev)
		{
			case LOG_LEVEL_FATAL : oss << "FATAL"; break;
			case LOG_LEVEL_ERROR : oss << "ERROR"; break;
			case LOG_LEVEL_WARN  : oss << "WARN" ; break;
			case LOG_LEVEL_INFO  : oss << "INFO" ; break;
			case LOG_LEVEL_DEBUG : oss << "DEBUG"; break;
		}
		oss << "\", \"logger\":\"" << name_ << "\", \"timestamp\":\"" << now() <<  "\", ";

		oss << "\"msg\":\"";
		log_printf(oss,msg,args...);
		oss << "\", ";
        oss << "\"function\":\"" << func << "\", ";
		oss << "\"file\":\"" << file << "\", ";
		oss << "\"line\":" << line << " ";
		oss << "}\n"; 

        logger_->log(oss.str());
	}

    void log()
    {} // no op

	template<class ... Args>
	void operator()(int sev , const char* func, int line, const char* file, const char* msg, Args ... args)
	{
        log(sev,func,line,file,msg,args...);
    }

    void operator()()
    {} // no op

private:

	std::string name_;
	std::shared_ptr<Logging> logger_;
};

}

#endif
#endif

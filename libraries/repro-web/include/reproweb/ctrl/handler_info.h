#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_HANDLER_INFO_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_HANDLER_INFO_DEF_GUARD_

#include <regex>
#include <iostream>

#include "reproweb/ctrl/handler.h"
#include "priohttp/common.h"
#include "reprocpp/ex.h"

 
namespace reproweb  {


	
class HandlerInfo
{
public:
    
    HandlerInfo( const std::string& m,  const std::string& p, http_handler_t handler );
    
    bool match(const std::string& m, const std::string& p, prio::patharguments_t& result);

    void invoke(prio::Request& req, prio::Response& res);

    std::vector<std::string> args() 	{ return args_; };
    std::string method() 				{ return method_; };
    std::regex regex() 					{ return path_regex_; };
        
private:
    std::string method_;
    std::regex path_regex_;    
    std::vector<std::string> args_;
    http_handler_t handler_;
};
   


class HttpFilterInfo
{
public:
    
    HttpFilterInfo( const std::string& m,  const std::string& p, http_filter_t filter, int priority_ = 0 );
    
    bool match(const std::string& m, const std::string& p);
    
    http_filter_t& filter() 	{ return filter_; }
    int priority() 				{ return priority_; };

private:

    std::regex method_regex_;  
    std::regex path_regex_;    
    http_filter_t filter_;
    int priority_;
};

class ExHandlerInfo
{
public:

	virtual ~ExHandlerInfo() {}
	virtual bool match(const std::exception&) = 0;
	virtual bool isa(const std::exception&) = 0;
	virtual void invoke(const std::exception& ex, prio::Request& req, prio::Response& res) = 0;

};

template<class E>
class ExceptionHandlerInfo : public ExHandlerInfo
{
public:

	typedef std::function<void(
		const E& ex,
		prio::Request& req,
		prio::Response& res
	)>
	ex_handler_t;

	ExceptionHandlerInfo(ex_handler_t h) 
		: handler_(h) 
	{}

	virtual bool match(const std::exception& ex)
	{
		return typeid(ex) == typeid(E);
	}

	virtual bool isa(const std::exception& ex)
	{
		return dynamic_cast<const E*>(&ex) != nullptr;
	}

	virtual void invoke(const std::exception& ex, prio::Request& req, prio::Response& res)
	{
		handler_(dynamic_cast<const E&>(ex), req, res);
	}

private:

	ex_handler_t handler_;
};


} // end namespace csgi
 
#endif


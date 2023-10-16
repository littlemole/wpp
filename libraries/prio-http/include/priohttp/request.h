#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_DEF_GUARD_

//! \file request.h
#include "priohttp/common.h"
#include "priohttp/arg.h"
#include "priohttp/path.h"
#include "priohttp/header.h"
#include "priohttp/attr.h"
#include "priohttp/queryparams.h"
#include "priocpp/api.h"
 
#include <regex>
   
//////////////////////////////////////////////////////////////

namespace prio  {

class HttpRequestParser;
class Conversation;

//! \brief HTTP Request
class Request
{
friend class HttpRequestParser;

public:

    //! HTTP Path information
	PathInfo path;
    //! HTTP Header information
	Headers headers;
    //! custom attributes 
	Attributes attributes;

    //! \private
	Request();
    //! \private
    Request(Conversation* con);
        
    //! HTTP body as a std::string
    const std::string& body() const noexcept;
    //! \private    
    Connection::Ptr con()  const noexcept;
    //! \private
    bool keep_alive()  const noexcept;
    //! \private
    bool detached()  const noexcept;

    //! HTTP body size
    size_t size() const noexcept;
    //! return complete request as valid HTTP represenation
    std::string toString();

    //! get request param by string
    std::string operator[](const std::string& s);
    
    std::string common_name();
    
    std::string host();
    
protected:

    Conversation* con_;
    size_t size_;
    std::string body_;
    bool detached_;
};

//! \private
class HttpRequest : public Request
{
friend class HttpRequestParser;

public:

	HttpRequest();
	HttpRequest(Conversation* con);

    void action(const std::string& a);
    Request& header(const std::string& key, const std::string& value) ;
    void body(const std::string& b);
    
    void detach() noexcept;

    void reset() noexcept;

    bool match(const std::string& method, std::regex regex, std::vector<std::string> args);

    void size(size_t  s) { size_ = s; };

    bool parse(const std::string& h);

};


} // close namespaces

#endif


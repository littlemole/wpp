#ifndef INCLUDE_PROMISE_HTTP_PATH_H_
#define INCLUDE_PROMISE_HTTP_PATH_H_

//! \file path.h

#include "priohttp/common.h"
#include "priohttp/arg.h"
#include "priohttp/queryparams.h"


namespace prio  {

//! \brief HTTP Path information
//! 
//! each HTTP Request object will have path information attached.
class PathInfo
{
public:

    //! the HTTP action string for Requests, ie "GET /index.html HTTP/1.0"
	const std::string& action() const noexcept  {return action_; }
    //! return the HTTP protocol, ie HTTP/1.1
	const std::string& protocol() const noexcept  {return protocol_; }
    //! return HTTP method like GET/POST etc
    const std::string& method() const noexcept;
    //! return full url, that is path + query parameters
    const std::string& url() const noexcept;
    //! return path from url, ie no query paramaters
    std::string path() const noexcept;
    //! return the query paramaeter string
    std::string querystring() const noexcept;

    //! \private
    patharguments_t path_info()  const noexcept;

    //! return Query Paramaters
    QueryParams queryParams() const;
    //! HTTP path arguments, if parsed by router
    Args args() const noexcept;

    //! clear this path info object
    void reset() noexcept;
    

    //! \private
    void action(const std::string& a);
    //! \private
    void method(const std::string& m)   { method_ = m; };
    //! \private
    void path(const std::string& p)     { path_ = p; };
    //! \private
    void protocol(const std::string& p) { protocol_ = p; };

    //! \private
    void set(pathargument_t t)
    {
    	args_.push_back(t);
    }

    //! \private
    std::string operator()() { return path(); }

    //! \private
    bool parse(const std::string& h);

private:

    std::string action_;
    std::string method_;
    std::string path_;
    std::string protocol_;

    patharguments_t args_;
};


}


#endif /* INCLUDE_PROMISE_HTTP_PATH_H_ */

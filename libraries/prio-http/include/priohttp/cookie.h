#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_COOKIE_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_COOKIE_DEF_GUARD_

//! \file cookie.h

#include "priohttp/common.h"

namespace prio  {

//! \brief HTTP Cookie class
class Cookie
{
public:
    //! construct empty cookie
    Cookie();
    //! construct simple cookie with name and value
    Cookie(const std::string& name, const std::string& value);

    //! set the cookie name
    Cookie& name(const std::string& value);
    //! set the cookie value
    Cookie& value(const std::string& value);    
    //! set expires value for cookie
    Cookie& expires(const std::string& value);
    //! set maxAge value in seconds
    Cookie& maxAge(int seconds);    
    //! set the cookie domain
    Cookie& domain(const std::string& value);
    //! set the cookie path
    Cookie& path(const std::string& value);
    //! make HTTPS only cookie
    Cookie& secure();
    
    //! return cookie name
    std::string name() const;
    //! return cookie value
    std::string value() const;
    //! return cookie expiration
    std::string expires() const;
    //! return cookie maxAge
    int maxAge() const;
    //! return cookie domain
    std::string domain() const;
    //! return cookie path
    std::string path() const;    
    //! return true if it is a HTTPS cookie
    bool isSecure() const;
        
    //! return cookie value suitable for HTTP set-cookie header
    std::string str() const;
    
private:
    std::string name_;
    std::string value_;
    std::string expires_;
    std::string maxAge_;    
    std::string domain_;
    std::string path_;
    bool secure_;
};

//! \brief Cookie Jar
class Cookies
{
public:

    //! parse a single Cookie from text string
    Cookie parseCookie(const std::string& txt);
    //! parse an incoming Cookies HTTP header
    std::vector<Cookie>& parse(const std::string& txt);

    //! check whether there is a cookie of given name
    bool exists(const std::string& name) const;
    //! return cookie by name
    const Cookie& get(const std::string& name) const;
    
    //! return all Cookies as a std::vector<Cookie>
    const std::vector<Cookie>& all() const;

    //! add a single Cookie
    Cookies& add(const Cookie& c);

    //! check whether there are any cookies avail
    bool empty() const { return cookies_.empty(); }

    //! clear cookie jar
    void clear();

    //! remove a named cookie from jar
    void remove(const std::string& name);

private:
    std::vector<Cookie> cookies_;
};

} // close namespaces

#endif

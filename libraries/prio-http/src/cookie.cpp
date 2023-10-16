
#include "priohttp/cookie.h"
#include <cstring>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif


namespace prio  {


Cookie::Cookie()
    :secure_(false)
{}

Cookie::Cookie(const std::string& name, const std::string& value)
    : name_(name), value_(value), secure_(false)
{}    

Cookie& Cookie::maxAge(int seconds)
{
    std::ostringstream oss;
    oss << seconds;
    maxAge_ = oss.str();
    return *this;
}  


Cookie& Cookie::name(const std::string& value)
{
    name_ = value;
    return *this;
}

Cookie& Cookie::value(const std::string& value)
{
    value_ = value;
    return *this;
}

Cookie& Cookie::expires(const std::string& value)
{
    expires_ = value;
    return *this;
}

Cookie& Cookie::domain(const std::string& value)
{
    domain_ = value;
    return *this;
}

Cookie& Cookie::path(const std::string& value)
{
    path_ = value;
    return *this;
}

Cookie& Cookie::secure()
{
    secure_ = true;
    return *this;
}    

std::string Cookie::name() const
{
    return name_;
}

std::string Cookie::value() const
{
    return value_;
}

std::string Cookie::expires() const
{
    return expires_;
}


int Cookie::maxAge() const
{
    std::istringstream iss(maxAge_);
    int result = 0;
    iss >> result;
    return result;
}

std::string Cookie::domain() const
{
    return domain_;
}

std::string Cookie::path() const
{
    return path_;
}

bool Cookie::isSecure() const
{
    return secure_;
}  

std::string Cookie::str() const
{
    std::ostringstream oss;
    oss << name_ << "=" << value_;
    if ( !expires_.empty() )
    {
        oss << ";expires=" << expires_;
    }
    if ( !maxAge_.empty() )
    {
        oss << ";max-Age=" << maxAge_;
    }    
    if ( !domain_.empty() )
    {
        oss << ";domain=" << domain_;
    }
    if ( !path_.empty() )
    {
        oss << ";path=" << path_;
    }
    if ( secure_ )
    {
        oss << ";secure";
    }
    return oss.str();
}

Cookie Cookies::parseCookie(const std::string& txt)
{
    Cookie cookie;
    std::vector<std::string> v = split(txt,';');
    if ( v.empty() )
    {
        return cookie;
    }
    
    std::vector<std::string> n = split(v[0],'=');
    if ( n.empty() )
    {
        return cookie;
    }
    
    cookie.name(n[0]);
    if ( n.size()>1)
    {
    	cookie.value(n[1]);
    }

    for ( size_t i = 1; i < v.size(); i++)
    {
        std::string s = v[i];
        if ( strcasecmp(s.c_str(),"secure") == 0 )
        {
            cookie.secure();
            continue;
        }
        
        std::vector<std::string> n = split(s,'=');
        if ( n.empty() )
        {
            continue;
        }     
               
        if ( strcasecmp(n[0].c_str(),"expires") == 0 )
        {
            cookie.expires(n[1]);
            continue;
        }
               
        if ( strcasecmp(n[0].c_str(),"max-age") == 0 )
        {
            std::istringstream iss(n[1]);
            int value = 0;
            iss >> value;
            cookie.maxAge(value);
            continue;
        }
                
        if ( strcasecmp(n[0].c_str(),"domain") == 0 )
        {
            cookie.domain(n[1]);
            continue;
        }
        
        if ( strcasecmp(n[0].c_str(),"path") == 0 )
        {
            cookie.path(n[1]);
            continue;
        }
    }
    return cookie;
}    

std::vector<Cookie>& Cookies::parse(const std::string& txt)
{
	cookies_.clear();
	std::vector<std::string> v = split(txt,' ');
	for ( size_t i = 0; i < v.size(); i++)
	{
		Cookie cookie = parseCookie(v[i]);
		cookies_.push_back(cookie);
	}
	return cookies_;
}

Cookies& Cookies::add(const Cookie& c) 
{ 
    remove(c.name());
    cookies_.push_back(c); 
    return *this; 
}


bool Cookies::exists(const std::string& name) const
{
    for(const Cookie& c : cookies_ ) {
        if( c.name() == name ) {
            return true;
        }
    }    
    return false;
}

const Cookie& Cookies::get(const std::string& name) const
{
    for(const Cookie& c : cookies_ ) {
        if( c.name() == name ) {
            return c;
        }
    }  
    
    static Cookie empty;
    return empty;
}


void Cookies::remove(const std::string& name)
{
    for( auto it = cookies_.begin(); it != cookies_.end(); it++ ) 
    {
        if( (*it).name() == name ) 
        {
            cookies_.erase(it);
            return;
        }
    }  
}

const std::vector<Cookie>& Cookies::all() const
{
    return cookies_;
}

void Cookies::clear()
{
	cookies_.clear();
}


} // close namespaces



#ifndef INCLUDE_PROMISE_HTTP_HEADER_H_
#define INCLUDE_PROMISE_HTTP_HEADER_H_

//! \file header.h

#include "priohttp/common.h"
#include "priocpp/common.h"
#include "priohttp/cookie.h"
 

namespace prio  {


//! \brief a Header value
//! 
//! could consist of multiple params
class HeaderValue
{
public:

    HeaderValue() {}
	HeaderValue(const std::string& s);

    //! return the single "main" param value if approp
	std::string main() const;
    //! access the param values as a map<std::string,std::string>
	std::map<std::string,std::string> params() const;

private:
	std::string value_;
};

//! \brief  collection of HeaderValue
//! 
//! each value could have multiple params
class HeaderValues
{
public:

    HeaderValues() {}
	HeaderValues(const std::string& val);

    //! return number of Header Values
	size_t size() const;
    //! return Header values at index
	const HeaderValue& operator[](size_t index) const;
    //! return the main value (=index zero)
	const HeaderValue& value() const;

private:
	std::vector<HeaderValue> values_;
};

typedef std::pair<std::string,std::string> header_t;
typedef std::vector<header_t> headers_t;

//! \brief Header class 
//! 
//! both HTTP Request and Response object will have a Headers member
class Headers
{
public:

    //! \private
    Headers()
    {}

    //! \private
    Headers(const std::vector<std::pair<std::string,std::string>>& h)
    {
        for ( auto& it : h)
        {
            set(it.first,it.second);
        }
    }    

    //! return all Headers as string suitable for HTTP transmission
    std::string toString() const;

    //! check whether a header of given key exists
    bool exists(const std::string& key) const noexcept;
    //! return header for given key as a simple string
    std::string get(const std::string& key) const noexcept;
    //! return HTTP accept header value
    std::string accept() const noexcept;
    //! return HTTP content-type header value
    std::string content_type() const noexcept;
    //! return HTTP content-ength header value
    size_t 		content_length() const;

    //! return Cookies as in this Cookie jar
    const Cookies& cookies() const noexcept;

    //! add a cookie
    Headers& cookie(const Cookie& c);

    //! \private
    bool keep_alive(const std::string& proto)  const noexcept;

    //! set arbitary HTTP header
    Headers& set(const std::string& key, const std::string& value) ;

    Headers& remove(const std::string& key) noexcept;

    //! clear HTTP headers
    void reset() noexcept;

    //! \private
    bool parse( std::istringstream& h);

    //! for headers with multiple values, access parsed header value for given key
    HeaderValues values(const std::string& key)
    {
    	return get(key);
    }

    //! \private
    std::vector<std::pair<std::string,std::string>>& raw()
    {
        return headers_;
    }

private:

    std::vector<std::pair<std::string,std::string>> headers_;
    Cookies cookies_;
};



}

#endif /* INCLUDE_PROMISE_HTTP_HEADER_H_ */

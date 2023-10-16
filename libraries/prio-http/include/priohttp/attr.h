#ifndef INCLUDE_PROMISE_HTTP_ATTR_H_
#define INCLUDE_PROMISE_HTTP_ATTR_H_

//! \file attr.h

#include "priohttp/common.h"
#include <any>

namespace prio  {


//! \brief Generic Attributes 
//! 
//! attached to HTTP request/response
//! stores attributes as std::any objects
class Attributes
{
public:

    //! cgeck whether attribute exists
    bool exists( const std::string& key ) const noexcept;
    std::any get( const std::string& key ) const;

	//! fetch attribute of type T for the given key
    template<class T>
    T attr(const std::string& key) const
    {
    	try
    	{
            return std::any_cast<T>(get(key));
    	}
    	catch(...)
    	{
    		throw repro::Ex("key not found");
    	}
    }

    //! set attribute as std::any object
    void set( const std::string& key, const std::any& a );

	//! set object a of tyoe T as attribute under given key
    template<class T>
	void set(const std::string& key, T a )
	{
    	set( key, std::any(a) );
	}

	//! clear attributes
    void reset();

private:
    std::map<std::string,std::any> attrs_;
};

}



#endif /* INCLUDE_PROMISE_HTTP_ATTR_H_ */

#ifndef _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_SQL_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_SQL_DEF_GUARD_

#include <repromysql/mysql-bindings.h>
#include <repromysql/mysql-async.h>
#include <metacpp/meta.h>

namespace reproweb {


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, std::string& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));

        if( !rv.null() )
        {
            t = r->field(std::string(name)).getString(); 
        }
        else
        {
            t = "";
        }    
    }
    catch(...)
    {
        t = "";
    }
}

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, int& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));
        if( !rv.null() )
        {
            t = r->field(std::string(name)).getInt(); 
        }
        else
        {
            t = 0;
        }
    }
    catch(...)
    {
        t = 0;
    }
}

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, double& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));
        if( !rv.null() )
        {
            t = r->field(std::string(name)).getDouble(); 
        }
        else
        {
            t = 0;
        }       
    }
    catch(...)
    {
        t = 0;
    }
} 

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, float& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));
        if( !rv.null() )
        {
            t = r->field(std::string(name)).getFloat(); 
        }            
        else
        {
            t = 0;
        }
    }
    catch(...)
    {
        t = 0;
    }

}

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, bool& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));
        if( !rv.null() )
        {
            t = r->field(std::string(name)).getInt() != 0; 
        }         
        else
        {
            t = 0;
        }
    }
    catch(...)
    {
        t = 0;
    }   
}

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, long long& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));
        if( !rv.null() )
        {
            t = r->field(std::string(name)).getLongLong(); 
        }  
        else
        {
            t = 0;
        }        
    }
    catch(...)
    {
        t = 0;
    }  
}

inline void fromSQL( const char* name, repromysql::result_async::Ptr r, char& t)
{
    try
    {
        const repromysql::Retval& rv = r->field(std::string(name));
        if( !rv.null() )
        {
            t = r->field(std::string(name)).getInt(); 
        }
        else
        {
            t = 0;
        }        
    }
    catch(...)
    {
        t = 0;
    }
}

template<class T>
void fromSQL(const char* name, repromysql::result_async::Ptr r, std::vector<T>& t)
{
    // do nothing
}

template<class T>
void fromSQL(repromysql::result_async::Ptr r, T& t);

template<class T>
void fromSQL(repromysql::result_async::Ptr r, std::vector<T>& v);


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


template<class T>
void fromSQL(repromysql::result_async::Ptr r, T& t)
{
    if( r->fetch() )
    {
        auto visitor = [&r]( auto name, auto m)
        {	
            typename decltype(m)::setter_value_t value;
            fromSQL(name.name,r,value);
            m = value;
        };
        meta::visit(t,visitor);
    }
}

template<class T>
void fromSQL(repromysql::result_async::Ptr r, std::vector<T>& v)
{
	v.clear();
    while( r->fetch() )
    {
        T t;
        auto visitor = [&r]( auto name, auto m)
        {	
            typename decltype(m)::setter_value_t value;

            fromSQL(name.name,r,value);
            m = value;
        };
        meta::visit(t,visitor);        

        v.push_back(std::move(t));
    }
}


}

#endif


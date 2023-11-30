#ifndef _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_SQL_LITE_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_SQL_LITE_DEF_GUARD_

#include <reprosqlite/sqlite.h>
#include <metacpp/meta.h>

namespace reproweb {


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

inline int sqlite_column(const char* name, reprosqlite::Result& r)
{
    for(size_t i = 0; i < r.cols(); i++)
    {
        if( r.columns[i] == name) return i;
    }
    return -1;
}

inline void fromSQL( const char* name, reprosqlite::Result& r, std::vector<std::string>& row, std::string& t)
{
    int i = sqlite_column(name,r);
    if(i==-1)
    {
        t = "";
        return;
    } 
    t = row[i];
}

template<class T >
void fromSQL( const char* name, reprosqlite::Result& r, std::vector<std::string>& row, T& t, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr)
{
    int i = sqlite_column(name,r);
    if(i==-1)
    {
        t = 0;
        return;
    } 
    std::istringstream iss(row[i]);
    iss >> t;
}

template<class T>
void fromSQL(const char* name, reprosqlite::Result& r, std::vector<std::string>& row, std::vector<T>& t)
{
    // do nothing
}

template<class T>
void fromSQL(reprosqlite::Result& r, std::vector<std::string>& row, T& t);

template<class T>
void fromSQL(reprosqlite::Result&  r, std::vector<std::string>& row, std::vector<T>& v);

template<class T>
void fromSQL(const char* name, reprosqlite::Result& r, std::vector<std::string>& row, T& t, typename std::enable_if<std::is_class<T>::value>::type* = nullptr))
{
    fromSQL(r,row,t);
}




///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


template<class T>
void fromSQL( reprosqlite::Result& r, std::vector<std::string>& row, T& t)
{
    auto visitor = [&r,&row]( auto name, auto m)
    {	
        typename decltype(m)::setter_value_t value;
        fromSQL(name.name,r,row,value);
        m = value;
    };
    meta::visit(t,visitor);
}

template<class T>
void fromSQL( reprosqlite::Result& r, T& t)
{
    if(r.rows()<1)
        return;

    std::vector<std::string>& row = r.row(0);

    fromSQL(r,row,t);
}

template<class T>
void fromSQL( reprosqlite::Result& r, std::vector<T>& v)
{
	v.clear();

    for( auto& row : r.data )
    {
        T t;
        auto visitor = [&r,&row]( auto name, auto m)
        {	
            typename decltype(m)::setter_value_t value;

            fromSQL(name.name,r,row,value);
            m = value;
        };
        meta::visit(t,visitor);        

        v.push_back(std::move(t));
    }
}


}

#endif


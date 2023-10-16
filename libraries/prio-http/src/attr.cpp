#include "priohttp/attr.h"


namespace prio  {


void Attributes::set( const std::string& key,const std::any& a )
{
    attrs_.insert( std::pair<std::string,std::any>( key, a ) );
}

std::any Attributes::get( const std::string& key ) const
{
    if ( attrs_.count(key) == 0 )
    {
        return std::any(nullptr);
    }
    return attrs_.at(key);
}

bool Attributes::exists( const std::string& key ) const noexcept
{
    return attrs_.count(key) > 0;
}

void Attributes::reset()
{
	attrs_.clear();
}

}

#include "priohttp/queryparams.h"
#include "priohttp/urlencode.h"

#include <cstring>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

namespace prio  {

QueryParams::QueryParams()
{

}

QueryParams::QueryParams(const std::string& s)
{
    std::vector<std::string> v = split(s,'&');
    for ( size_t i = 0; i < v.size(); i++ )
    {
        if ( !v[i].empty() )
        {
            std::vector<std::string> p = split(v[i],'=');
            if ( p.size() > 1 )
            {
                params_.push_back(
                    std::make_pair(
                        Urlencode::decode(p[0]),
                        Urlencode::decode(p[1])
                    )
                );
            }
        }
    }
}


bool QueryParams::exists(const std::string& key) const
{
    for( std::size_t i = 0; i < params_.size(); i++)
    {
        if ( strcasecmp(params_[i].first.c_str(),key.c_str()) == 0 )
        {
            return true;
        }
    }
    
    return false;
}


std::string QueryParams::get(const std::string& key) const
{
    for( std::size_t i = 0; i < params_.size(); i++)
    {
        if ( strcasecmp(params_[i].first.c_str(),key.c_str()) == 0 )
        {
            return params_[i].second;
        }
    }
    return "";
} 


std::vector<std::string> QueryParams::array(const std::string& key) const
{
    std::vector<std::string> v;
    for( std::size_t i = 0; i < params_.size(); i++)
    {
        if ( strcasecmp(params_[i].first.c_str(),key.c_str()) == 0 )
        {
            v.push_back( params_[i].second);
        }
    }
    return v;
} 

void QueryParams::remove(const std::string& key)
{
    std::vector<std::pair<std::string,std::string>> tmp;
    for( std::size_t i = 0; i < params_.size(); i++)
    {
        if ( strcasecmp(params_[i].first.c_str(),key.c_str()) != 0 )
        {
            tmp.push_back(params_[i]);
        }
    }
    params_ = tmp;
} 

std::set<std::string> QueryParams::keys() const
{
	std::set<std::string> v;
	for ( auto it = params_.begin(); it != params_.end(); it++)
	{
		v.insert(it->first);
	}
	return v;
}

void QueryParams::set(const std::string& key, const std::string& value)
{
    remove(key);
    add(key,value);
}

void QueryParams::add(const std::string& key, const std::string& value)
{
    params_.push_back( std::make_pair( key, value ) );
}

std::string& QueryParams::operator[] (const std::string& key)
{
    for( std::size_t i = 0; i < params_.size(); i++)
    {
        if ( strcasecmp(params_[i].first.c_str(),key.c_str()) == 0 )
        {
            return params_[i].second;
        }
    }
    add(key,"");
    return params_.back().second;
}

std::string QueryParams::toString() const
{
        if(params_.size() == 0) return "";
    
	std::ostringstream oss;
	for ( auto it = params_.begin(); it != params_.end(); it++)
	{
		oss << (*it).first << "=" << Urlencode::encode( (*it).second ) << "&";
	}
	std::string tmp = oss.str();
	tmp.pop_back();
	return tmp;
}




} // close namespaces


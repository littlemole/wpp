#include "priohttp/header.h"
#include <sstream>
#include <cstring>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif


namespace prio  {


HeaderValue::HeaderValue(const std::string& s)
	: value_(s)
{}

std::string HeaderValue::main() const
{
	std::vector<std::string> items = split(trim(value_),';');
	if(items.empty()) return "";
	return items[0];
};

std::map<std::string,std::string> HeaderValue::params() const
{
	std::map<std::string,std::string> result;

	std::vector<std::string> items = split(trim(value_),';');

	if( items.size() < 2 ) return result;

	for ( size_t i = 1; i < items.size(); i++)
	{
		std::vector<std::string> pieces = split(trim(items[i]),'=');
		if ( pieces.size() > 1)
		{
			result[trim(pieces[0])] = unquote(trim(pieces[1]));
		}
	}
	return result;;
};



HeaderValues::HeaderValues(const std::string& val)
{
	std::vector<std::string> items = split(val,',');
	for ( auto& s : items)
	{
		values_.push_back( HeaderValue(s) );
	}
}

size_t HeaderValues::size() const
{
	return values_.size();
}

const HeaderValue& HeaderValues::operator[](size_t index) const
{
	static HeaderValue empty("");
	if ( values_.empty() || index >= values_.size()) return empty;

	return values_[index];
}

const HeaderValue& HeaderValues::value() const
{
	return operator[](0);
}



std::string Headers::toString() const
{
	std::ostringstream oss;
	for (std::pair<std::string,std::string> h : headers_ ) {
		oss << h.first << ":" << h.second << "\r\n";
	}
	oss << "\r\n";
	return oss.str();


}

const Cookies& Headers::cookies() const noexcept
{
    return cookies_;
}

Headers& Headers::cookie(const Cookie& c)
{
    cookies_.add(c);
    return *this;
}

bool Headers::keep_alive(const std::string& proto) const noexcept
{
	std::string conn = get("Connection");
	if ( conn.empty() )
	{
		if ( strcasecmp(proto.c_str(),"HTTP/1.1") == 0 )
		{
			return true;
		}
		return false;
	}

	if ( strcasecmp( conn.c_str(), "Close") == 0 )
	{
		return false;
	}

	if ( strcasecmp( conn.c_str(), "Keep-Alive") == 0 )
	{
		return true;
	}

	return false;
}

std::string Headers::content_type() const noexcept
{
    return get("CONTENT-TYPE");
}

size_t Headers::content_length() const
{
    std::string s = get("CONTENT-LENGTH");
    if ( s.empty() )
    {
        return 0;
    }

    std::istringstream iss(s);
    size_t result = 0;
    iss >> result;
    return result;
}

std::string Headers::accept() const noexcept
{
    return get("ACCEPT");
}

std::string Headers::get(const std::string& key) const noexcept
{
    for ( const std::pair<std::string,std::string>& header : headers_ )
    {
        if ( strcasecmp(header.first.c_str(), key.c_str()) == 0 )
        {
            return header.second;
        }
    }
    return "";
}

bool Headers::exists(const std::string& key) const noexcept
{
    for ( const std::pair<std::string,std::string>& header : headers_ )
    {
        if ( strcasecmp(header.first.c_str(), key.c_str()) == 0 )
        {
            return true;
        }
    }
    return false;
}


Headers& Headers::remove(const std::string& key) noexcept
{
	for ( auto it = headers_.begin(); it != headers_.end(); it++)
    {
        if ( strcasecmp((*it).first.c_str(), key.c_str()) == 0 )
        {
			headers_.erase(it);
            return *this;
        }
    }
    return *this;
}

Headers& Headers::set(const std::string& key, const std::string& val)
{
    headers_.push_back( header_t(key,val) );


    if ( strcasecmp(key.c_str(), "COOKIE") == 0 )
    {
    	cookies_.parse( get("COOKIE") );
    }

    return *this;
}

void Headers::reset() noexcept
{
	headers_.clear();
	cookies_.clear();
}



bool Headers::parse( std::istringstream& h)
{
	reset();

	while ( h )
	{
		std::string line;
		std::getline(h,line);
		if(line.empty())
		{
			break;
		}

		size_t pos = line.find(":");
		if ( pos == std::string::npos )
		{
			set( line, "" );
		}
		else
		{
			std::string key = trim(line.substr(0,pos));
			std::string val = trim(line.substr(pos+1));
			set( key, val );
		}
	}

	return false;
}



}





#include "priohttp/path.h"
#include "priohttp/urlencode.h"


namespace prio  {

void PathInfo::action(const std::string& a)
{
	action_ = a;
	auto v = split(a," ");
	if ( v.size()>1)
	{
		method_ = v[0];
		path_ = v[1];
	}
}

const std::string& PathInfo::method() const noexcept
{
    return method_;
}


std::string PathInfo::querystring() const noexcept
{
    size_t pos = path_.find("?");
    if ( pos == std::string::npos || pos >= path_.size()-1 )
    {
        return "";
    }

    return path_.substr(pos+1);
}

const std::string& PathInfo::url() const noexcept
{
    return path_;
}

std::string PathInfo::path() const noexcept
{
    size_t pos = path_.find("?");
    if ( pos == std::string::npos )
    {
        return path_;
    }

    return path_.substr(0,pos);
}

QueryParams PathInfo::queryParams() const
{
    return QueryParams(querystring());
}

patharguments_t PathInfo::path_info() const noexcept
{
    return args_;
}


Args PathInfo::args() const noexcept
{
    return Args(args_);
}

void PathInfo::reset() noexcept
{
	action_ = "";
	method_ ="";
	path_ = "";
	protocol_ = "";
	args_.clear();
}


bool PathInfo::parse(const std::string& line)
{
	size_t pos1 = line.find(" ");
	if ( pos1 == std::string::npos )
	{
		throw repro::Ex("invalid http action");
	}
	method( line.substr(0,pos1) );

	pos1 = line.find_first_not_of(" ",pos1);
	if ( pos1 == std::string::npos )
	{
		throw repro::Ex("invalid http action");
	}

	size_t pos2 = line.find(" ",pos1);
	if ( pos2 == std::string::npos )
	{
		throw repro::Ex("invalid http action");
	}
	path( line.substr(pos1,pos2-pos1) );

	pos2 = line.find_first_not_of(" ",pos2);
	if ( pos2 == std::string::npos )
	{
		throw repro::Ex("invalid http action");
	}

	protocol( line.substr(pos2));

	return false;
}


}






#include "priohttp/http_server.h"
#include "priohttp/request.h"
#include "priohttp/conversation.h"


namespace prio  {


Request::Request( )
    :
	  size_(0),
	  body_(""),
	  detached_(false)
{}

Request::Request( Conversation* c)
    : con_(c),
	  size_(0),
	  body_(""),
	  detached_(false)
{}

std::string Request::host()
{
    std::ostringstream oss;

    if( headers.exists("X-Forwarded-proto"))
    {
        oss << headers.get("X-Forwarded-proto");
    }
    else
    {
        if( attributes.attr<bool>("is_secure") )
        {
            oss << "https";
        }
        else{
            oss << "http";
        }
    }

    oss << "://";

    if( headers.exists("X-Forwarded-Host"))
    {
        oss << headers.get("X-Forwarded-Host");
    }
    else
    {
        oss << headers.get("Host");
    }

    return oss.str();
}


std::string Request::operator[](const std::string& s)
{
	// path param

	Args args = path.args();
	if ( args.exists(s) )
	{
		return args.get(s);
	}

	// query param
	QueryParams qp(path.queryParams());
	if (qp.exists(s))
	{
		return qp.get(s);
	}

	// cookie value

	const Cookies& c = headers.cookies();

	if(c.exists(s))
	{
		return c.get(s).value();
	}

	// header

	if(headers.exists(s))
	{
		return headers.values(s).value().main();
	}

	return "";
}



bool Request::keep_alive() const noexcept
{
	return headers.keep_alive(path.protocol());
}

const std::string& Request::body() const noexcept
{
    return body_;
}


Connection::Ptr Request::con() const noexcept
{
    return con_->con();
}


bool Request::detached() const noexcept
{
	 return detached_;
}


size_t Request::size() const noexcept
{
	 return size_;
}


std::string Request::toString()
{
    if ( path.action().empty() )
    {
        throw repro::Ex("invalid request");
    }

    if ( !body_.empty() && headers.get("CONTENT-LENGTH") == "")
    {
    	std::ostringstream oss;
    	oss << body_.size();
    	HttpRequest& req = (HttpRequest&)*this;
    	req.header("CONTENT-LENGTH",oss.str());
    }

	std::ostringstream oss;
	oss << path.action() << "\r\n";
	oss << headers.toString();
	oss << body_;

	return oss.str();
}

std::string Request::common_name()
{
	if(!con_) return "";

	return con_->common_name();
}




HttpRequest::HttpRequest( )
{}

HttpRequest::HttpRequest( Conversation* s)
    : Request(s)
{}


void HttpRequest::action(const std::string& a)
{
	path.action(a);
}

void HttpRequest::body(const std::string& b)
{
	body_ = b;
}


void HttpRequest::reset() noexcept
{
	size_ = 0;
	headers.reset();
	body_ = "";
	attributes.reset();
	path.reset();
}


bool HttpRequest::match(const std::string& method, std::regex rgx, std::vector<std::string> args)
{
    if ( path.method() != method )
    {
        return false;
    }

    std::smatch match;
	std::string p = path.path();
    if ( !std::regex_match(p,match,rgx) )
    {
        return false;
    }

    for ( size_t i = 0; i < (match.size()-1); i++)
    {
    	std::string m = match[i+1];
    	path.set( pathargument_t(args[i],m) );
    }
    return true;
}



Request& HttpRequest::header(const std::string& key, const std::string& val)
{
    headers.set( key,val );
    return *this;
}



void HttpRequest::detach() noexcept
{
	detached_ = true;
}

bool HttpRequest::parse(const std::string& h)
{
	size(0);

	std::istringstream iss_headers(h);
	std::string line;
	std::getline(iss_headers,line);
	action(line);

	path.parse(line);

	headers.parse(iss_headers);

	size_t len = headers.content_length();
	if ( len > 0 )
	{
		size(len);
		return true;
	}
	return false;
}




} // close namespaces


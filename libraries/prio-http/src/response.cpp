#include "priohttp/conversation.h"
#include "priohttp/http_server.h"
#include "priohttp/response.h"
#include <cryptoneat/cryptoneat.h>
#include <sstream>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

namespace prio {


Response::Response(Conversation* rp)
    : http_(rp),status_code_(500),size_(0),headersSent_(false),isChunked_(false),isGzipped_(false)
{
}




Response& Response::contentType(const std::string& val)
{
    headers.set("content-type",val );
    return *this;
}       

Response& Response::header(const std::string& key, const std::string& val)
{
    headers.set(key,val);
    return *this;
}

Response& Response::body(const std::string& b)
{
    if ( !b.empty() )
    {
   		body_ = b;
    }
    return *this;
}

Response& Response::cookie(const Cookie& c)
{
	headers.cookie(c);
    return *this;
}

repro::Future<> Response::flush()
{
	if(http_)
	{
		return http_->flush(*this);
	}

    auto p = repro::promise();
    nextTick([p]()
    {
        p.resolve();
    });
    return p.future();
}




Response& Response::status(const std::string& s)
{
    status_ = s;

    std::vector<std::string> v = split(s,' ');
    if(v.size() < 2)
        return *this;

    proto_ = v[0];

    std::istringstream iss(v[1]);
    iss >> status_code_;

    return *this;
}

Response& Response::ok()
{
    return status("HTTP/1.1 200 OK");
}

Response& Response::error()
{
    return status("HTTP/1.1 500 internal error");
}

Response& Response::bad_request()
{
    return status("HTTP/1.1 400 bad request");
}

Response& Response::unauthorized()
{
    return status("HTTP/1.1 401 unauthorized");
}

Response& Response::forbidden()
{
    return status("HTTP/1.1 403 forbidden");
}

Response& Response::not_found()
{
    return status("HTTP/1.1 404 not found");
}

Response& Response::redirect(const std::string& s, int code)
{
    headers.set("location",s);
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code;
    
    return status(oss.str());
}

Response& Response::redirect(Request& req, const std::string& s, int code)
{
    std::ostringstream oss;

    if( req.headers.exists("X-Forwarded-proto"))
    {
        oss << req.headers.get("X-Forwarded-proto");
    }
    else
    {
        if( req.attributes.attr<bool>("is_secure") )
        {
            oss << "https";
        }
        else{
            oss << "http";
        }
    }

    oss << "://";

    if( req.headers.exists("X-Forwarded-Host"))
    {
        oss << req.headers.get("X-Forwarded-Host");
    }
    else
    {
        oss << req.headers.get("Host");
    }

    oss << s;

    return redirect(oss.str(),code);
}

const std::string& Response::status() const noexcept
{
    return status_;
}


const std::string& Response::proto() const noexcept
{
    return proto_;
}


int Response::statusCode() const noexcept
{
    return status_code_;
}


const std::string& Response::body() const noexcept
{
    return body_;
}

std::string Response::toString()
{
    if ( status_.empty() )
    {
        status("HTTP/1.0 500 Error");
    }

	std::ostringstream oss;
	oss << status_;
	oss << "\r\n";
	oss << headers.toString();
	oss << body_;

	return oss.str();
}

size_t Response::size() const noexcept
{
	 return size_;
}


void Response::statusCode(int i)
{
	status_code_ = i;
}

void Response::proto(const std::string& p)
{
	proto_ = p;
}

void Response::chunk(const std::string& ch)
{
	isChunked_ = true;

	http_->chunk(ch);
}

Response& Response::gzip()
{
	if(isGzipped_ == false)
	isGzipped_ = true;
	return *this;
}


HttpResponse::HttpResponse(Conversation* rp)
    : Response(rp)
{
}


void HttpResponse::size(size_t s)
{
	 size_ = s;
}

void HttpResponse::flushHeaders()
{
	if(headersSent_) return;

    std::string content_len = headers.get("Content-Length");
    if ( content_len.empty() && !isChunked_ )
    {
        std::string key = "Content-Length";
        std::ostringstream oss;
        oss << body_.size();
        std::string val = oss.str();
        headers.set(key,val);
    }
    if( headers.get("Content-Length") == "-")
    {
        headers.remove("Content-Length");
    }

    if( isChunked_ )
    {
    	headers.set("TRANSFER-ENCODING","chunked");
    }
    if( isGzipped_ )
    {
    	headers.set("CONTENT-ENCODING","gzip");
    }

    if ( http_ )
    {
        if (http_->keepAlive())
		{
			std::string c = headers.get("Connection");
			if ( c == "" )
			{
				headers.set("Connection","Keep-Alive");
			}
		}
    }
    if ( !headers.cookies().empty())
    {
    	const std::vector<Cookie>& cookies = headers.cookies().all();
        for ( size_t i = 0; i < cookies.size(); i++ )
        {
            std::string key = "Set-Cookie";
            std::string val = cookies[i].str();
            headers.set(key,val);
        }
    }

    headersSent_ = true;
}

HttpResponse& HttpResponse::reset()
{
    status_ = "";
    body_ = "";
    headers.reset();
    isChunked_ = false;
    isGzipped_ = false;
    headersSent_ = false;
    return *this;
}


void HttpResponse::onCompletion(std::function<void(Request& req, Response& res)> f)
{
	http_->onCompletion(f,*this);
}


void HttpResponse::onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f)
{
	http_->onFlushHeaders(f,*this);
}

} // close namespaces



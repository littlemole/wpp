#include "priohttp/conversation.h"
#include "priocpp/api.h"
#include "priohttp/server/reader.h"

namespace prio  {

ServerHttpReader::ServerHttpReader(ReaderWriterConversation* c)
	:con_(c)
{}

void ServerHttpReader::read()
{
	con_->read()
	.then( [this](std::string s)
	{
		consume(s);
	})
	.otherwise( [this](const std::exception_ptr& ex)
	{
		con_->onRequestError(ex);
	});
}


HttpHeaderReader::HttpHeaderReader(ReaderWriterConversation* c)
	: ServerHttpReader(c)
{

}

HttpHeaderReader::~HttpHeaderReader()
{}

void HttpHeaderReader::consume(const std::string& s)
{
    headers_stream_.append( s );


    size_t pos = headers_stream_.find("\r\n\r\n");
    if ( pos != std::string::npos)
    {
    	size_t len = headers_stream_.size()-(4+pos);
		if ( len > 0 )
		{
			body_stream_.append( headers_stream_.c_str()+pos+4, len );
		}
		headers_stream_.erase( pos );

		try
		{
			parse_headers();
			con_->onHeadersComplete(body_stream_);
		}
		catch(const std::exception_ptr& ex)
		{
			con_->onRequestError(ex);
		}
		return;
    }

    read();
}

bool HttpHeaderReader::parse_headers()
{
	HttpRequest& req = (HttpRequest&)(con_->request());

	return req.parse(headers_stream_);
}




HttpContentLengthBodyReader::HttpContentLengthBodyReader(ReaderWriterConversation* c)
    : ServerHttpReader(c),size_(c->request().size())
{
}

void HttpContentLengthBodyReader::consume(const std::string& c)
{
	body_.append(c);

	if ( body_.size() > size_ )
	{
		body_.erase(size_);
	}

	if ( body_.size() >= size_ )
	{
		con_->onRequestComplete(body_);
		return;
	}

	read();
}



} // close namespaces


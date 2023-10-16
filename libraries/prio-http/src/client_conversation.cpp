#include "priocpp/api.h"
#include "priohttp/client_conversation.h"
#include "priohttp/http2_client.h"

namespace prio  {



class ClientHttpReader
{
public:

	ClientHttpReader(ReaderWriterConversation* c);

	virtual ~ClientHttpReader() {}

	virtual void consume(const std::string& s) = 0;
	virtual void add(const std::string& s) { body.append(s); };
	virtual void complete(const std::string& c) = 0;

	std::string body;

	ReaderWriterConversation* con() { return con_; }

protected:

	void read();

	ReaderWriterConversation* con_;
    std::unique_ptr<ClientHttpReader> body_reader_;

};


class ClientHttpHeaderReader : public ClientHttpReader
{
public:


	ClientHttpHeaderReader(ReaderWriterConversation* c);

	virtual ~ClientHttpHeaderReader();

	virtual void consume(const std::string& s);
	virtual void complete(const std::string& c);

private:

	void parseHeaders();
	bool parse_headers();


	std::string headers_stream_;
	std::string body_stream_;
};


class HttpClientContentLengthBodyReader : public ClientHttpReader
{
public:

	HttpClientContentLengthBodyReader(ClientHttpReader* wrapped);

	virtual ~HttpClientContentLengthBodyReader()
	{}

	virtual void consume(const std::string& c);
	virtual void complete(const std::string& c);

protected:
	size_t size_;
	std::unique_ptr<ClientHttpReader> wrapped_;

};


class HttpClientChunkedBodyReader : public ClientHttpReader
{
public:

	HttpClientChunkedBodyReader(ClientHttpReader* wrapped);

	virtual ~HttpClientChunkedBodyReader()
	{}

	virtual void consume(const std::string& c);
	virtual void complete(const std::string& c);

protected:

	size_t hasChunkHeader();
	size_t chunkSize(size_t pos);
	bool isChunkedMsgDone();
	size_t size_;
	std::string body_stream_;

	std::unique_ptr<ClientHttpReader> wrapped_;

};

class HttpClientPlainBodyReader : public ClientHttpReader
{
public:

	HttpClientPlainBodyReader(ReaderWriterConversation* c);

	virtual ~HttpClientPlainBodyReader()
	{}

	virtual void consume(const std::string& c) {}
	virtual void complete(const std::string& c);

protected:

};


class HttpClientGzippedBodyReader : public ClientHttpReader
{
public:

	HttpClientGzippedBodyReader(ReaderWriterConversation* c);


	virtual ~HttpClientGzippedBodyReader()
	{}

	virtual void consume(const std::string& c) {};
	virtual void add(const std::string& c);
	virtual void complete(const std::string& c);

protected:

};


ClientHttpReader::ClientHttpReader(ReaderWriterConversation* c)
	:con_(c)
{}

void ClientHttpReader::read()
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


ClientHttpHeaderReader::ClientHttpHeaderReader(ReaderWriterConversation* c)
	: ClientHttpReader(c)
{}

ClientHttpHeaderReader::~ClientHttpHeaderReader()
{}

void ClientHttpHeaderReader::consume(const std::string& s)
{
    headers_stream_.append( s );

    size_t pos = headers_stream_.find("\r\n\r\n");
    if ( pos != std::string::npos)
    {
		int len = headers_stream_.size()-(4+pos);
		if ( len > 0 )
		{
			body_stream_.append( s.c_str()+pos+4, len );
		}
		headers_stream_.erase( pos );

		parseHeaders();

		return;
    }

    read();
}

void ClientHttpHeaderReader::complete(const std::string& c)
{

}


void ClientHttpHeaderReader::parseHeaders()
{
	try
	{
		parse_headers();
		if (this->con_->response().status().substr(0, 3) == "100")
		{
			
			headers_stream_ = "";
			std::string tmp = body_stream_;
			body_stream_ = "";
			this->consume(tmp);
			return;
		}
		con_->onHeadersComplete(body_stream_);
	}
	catch(...)
	{
		con_->onRequestError(std::current_exception());
	}
}

bool ClientHttpHeaderReader::parse_headers()
{
	HttpResponse& response = (HttpResponse&)(con_->response());
    response.size(0);
    std::istringstream iss_headers(headers_stream_);
    std::string line;
    std::getline(iss_headers,line);
    response.status(trim( line));

    size_t pos1 = response.status().find(" ");
    if ( pos1 == std::string::npos )
    {
    	throw repro::Ex("invalid status line");
    }
    response.proto( response.status().substr(0,pos1));

    pos1 = response.status().find_first_not_of(" ",pos1);
    if ( pos1 == std::string::npos )
    {
    	throw repro::Ex("invalid status line");
    }

    size_t pos2 = response.status().find(" ",pos1);
    if ( pos2 == std::string::npos )
    {
    	throw repro::Ex("invalid status line");
    }
    std::istringstream iss(response.status().substr(pos1,pos2-pos1));
    int s;
    iss >> s;
    response.statusCode(s);

    response.headers.reset();
    while ( iss_headers )
    {
	    std::string line;
	    std::getline(iss_headers,line);
	    if(line.empty())
	    {
	        break;
	    }

	    size_t pos = line.find(":");
	    if ( pos == std::string::npos )
	    {
	    	response.headers.set( line, "" );
	    }
	    else
	    {
	        std::string key = trim(line.substr(0,pos));
	        std::string val = trim(line.substr(pos+1));
	        response.headers.set( key, val );
	    }
    }

    std::string te = response.headers.get("TRANSFER-ENCODING");
    if ( !te.empty() )
    {
    	if ( response.headers.values("TRANSFER-ENCODING").value().main() == "chunked")
    	{
    		response.isChunked(true);
    		return true;
    	}
    }

    std::string size = response.headers.get("CONTENT-LENGTH");
    if ( !size.empty() )
    {
		std::istringstream iss(size);
		size_t s = 0;
		iss >> s;
		response.size(s);

		return true;
    }

	return false;
}



HttpClientContentLengthBodyReader::HttpClientContentLengthBodyReader(ClientHttpReader* wrapped)
    : ClientHttpReader(wrapped->con()),
	  size_(wrapped->con()->response().size()),
	  wrapped_(wrapped)
{
}

void HttpClientContentLengthBodyReader::consume(const std::string& c)
{
	wrapped_->add(c);

	if ( wrapped_->body.size() > size_ )
	{
		wrapped_->body.erase(size_);
	}

	if ( wrapped_->body.size() >= size_ )
	{
		wrapped_->complete(wrapped_->body);
		return;
	}

	read();
}

void HttpClientContentLengthBodyReader::complete(const std::string& c)
{
	wrapped_->complete(c);
}


HttpClientChunkedBodyReader::HttpClientChunkedBodyReader(ClientHttpReader* wrapped)
    : ClientHttpReader(wrapped->con()),
	  size_(wrapped->con()->response().size()),
	  wrapped_(wrapped)
{
}

void HttpClientChunkedBodyReader::complete(const std::string& c)
{
	wrapped_->complete(c);
}

void HttpClientChunkedBodyReader::consume(const std::string& c)
{
	body_stream_.append(c);

	size_t pos = hasChunkHeader();
	if(pos == std::string::npos)
	{
		read();
		return;
	}

	size_t s = chunkSize(pos);

	if( s == 0)
	{
		if(isChunkedMsgDone())
		{
			con_->onResponseComplete(wrapped_->body);
		}
		else
		{
			read();
		}
		return;
	}

	if ( body_stream_.size()-pos-2 >= s)
	{
		wrapped_->add(body_stream_.substr(pos+2,s));

		body_stream_ = body_stream_.substr(pos+2+s+2);
		if(body_stream_.empty())
		{
			read();
		}
		else
		{
			consume("");
		}
		return;
	}

	read();
}

bool HttpClientChunkedBodyReader::isChunkedMsgDone()
{
	size_t pos = body_stream_.find("\r\n\r\n");

	if( pos == std::string::npos)
		return false;

	return true;
}

size_t HttpClientChunkedBodyReader::hasChunkHeader()
{
	size_t pos = body_stream_.find("\r\n");
	return pos;
}

size_t HttpClientChunkedBodyReader::chunkSize(size_t pos)
{
	std::istringstream iss( body_stream_.substr(0,pos));
	size_t s;
	iss >> std::hex >> s;

	return s;
}



HttpClientPlainBodyReader::HttpClientPlainBodyReader(ReaderWriterConversation* c)
    : ClientHttpReader(c)
{
}

void HttpClientPlainBodyReader::complete(const std::string& c)
{
	con_->onResponseComplete(c);
}

HttpClientGzippedBodyReader::HttpClientGzippedBodyReader(ReaderWriterConversation* c)
    : ClientHttpReader(c)
{
}


void HttpClientGzippedBodyReader::add(const std::string& c)
{
	Decompressor d;
	d.decompress(c);
	body.append(d.flush());
}

void HttpClientGzippedBodyReader::complete(const std::string& c)
{
	Decompressor d;
	d.decompress(c);
	con_->onResponseComplete(d.flush());
}

///////////////////////////////////////////////////////////////////////////




HttpClientConversation::HttpClientConversation(Connection::Ptr client,Request& req)
	: req(req),
	  res(0),
	  con_(client),
	  keep_alive_(false)
{
	reader_.reset( new ClientHttpHeaderReader(this));
	REPRO_MONITOR_INCR(HttpCLientConversation);
}
 
HttpClientConversation::~HttpClientConversation()
{
	REPRO_MONITOR_DECR(HttpCLientConversation);
}

prio::Callback<Request&,Response&>& HttpClientConversation::on(Connection::Ptr client,Request& req)
{
	auto con = new HttpClientConversation(client,req);
	auto r = Ptr(con);
	r->self_ = r;

	client->write(req.toString())
	.then( [con](Connection::Ptr s)
	{
		con->reader_->consume("");
	})
	.otherwise( [con](const std::exception_ptr& ex)
	{
		con->onRequestError(ex);
	});

	return r->cb_;
}


std::string HttpClientConversation::common_name()
{
	return con_->common_name();
}


void HttpClientConversation::onHeadersComplete(const std::string& b)
{
	bool readBody = res.isChunked() || res.size() > 0;
	if(readBody)
	{
		std::string s = b;

		ClientHttpReader* r = nullptr;
		if ( res.isGzipped())
		{
			r = new HttpClientGzippedBodyReader(this);
		}
		else
		{
			r = new HttpClientPlainBodyReader(this);
		}

		if(res.isChunked())
		{
			reader_.reset( new HttpClientChunkedBodyReader(r));
		}
		else
		{
			reader_.reset( new HttpClientContentLengthBodyReader(r));
		}
		reader_->consume(s);
	}
	else
	{
		onResponseComplete(b);
	}
}


void HttpClientConversation::onResponseComplete(const std::string& b)
{
	res.body( b );

	cb_.resolve(req,res);

	((HttpRequest&)req).reset();
	((HttpResponse&)res).reset();

	con_->close();

	self_.reset();
}

void HttpClientConversation::onRequestError(const std::exception_ptr& ex)
{
	cb_.reject(ex);
	self_.reset();
}


void HttpClientConversation::onRequestComplete(const std::string& b)
{
}


repro::Future<std::string> HttpClientConversation::read()
{
	auto p = repro::promise<std::string>();

	con_->read()
	.then( [p] (Connection::Ptr c, std::string s)
	{
		p.resolve(s);
	})
	.otherwise([p](const std::exception_ptr& ex)
	{
		p.reject(ex);
	});

	return p.future();
}

repro::Future<> HttpClientConversation::write(const std::string& s)
{
	auto p = repro::promise<>();

	con_->write(s)
	.then( [p] (Connection::Ptr c)
	{
		p.resolve();
	})
	.otherwise([p](const std::exception_ptr& ex)
	{
		p.reject(ex);
	});

	return p.future();
}

Connection::Ptr HttpClientConversation::con()
{
	return con_;
}


//////////////////////////////////////////////////////////////////////////




Http2ClientConversation::Http2ClientConversation(Connection::Ptr client,Request& req)
	: req(req),
	con_(client),
	keep_alive_(false),
	http2_(new http2_client_session(this))
{
	REPRO_MONITOR_INCR(Http2CLientConversation);
}

Http2ClientConversation::~Http2ClientConversation()
{
	REPRO_MONITOR_DECR(Http2CLientConversation);
}

std::string Http2ClientConversation::common_name()
{
	return con_->common_name();
}

prio::Callback<Request&,Response&>& Http2ClientConversation::on(Connection::Ptr client,Request& req)
{
	auto con = new Http2ClientConversation(client,req);
	auto r = Ptr(con);
	r->self_ = r;

	r->http2_->initialize_nghttp2_session();
    r->http2_->send_connection_header();
    r->http2_->send()
    .then([r]()
    {		
		r->http2_->flush(r->req);
		return r->http2_->send();
	})		
    .then([r]()
    {
		r->schedule_read();
    })
    .otherwise([r](const std::exception_ptr& ex)
	{
		r->onRequestError(ex);
	});     

	return r->cb_;
}

void Http2ClientConversation::resolve(Request& req, Response& res)
{ 	
    cb_.resolve(req,res);
}

void Http2ClientConversation::schedule_read()
{
	auto ptr = shared_from_this();

    con_->read()
    .then([ptr](Connection::Ptr,std::string s)
    {
        return ptr->http2_->recv(s);
    })
    .then([ptr]()
    {
        ptr->schedule_read();
    })
    .otherwise([ptr](const std::exception_ptr& ex)
	{
		ptr->onRequestError(ex);
	}); 
}

void Http2ClientConversation::onRequestError(const std::exception_ptr& ex)
{
	cb_.reject(ex);
	self_.reset();
}


Connection::Ptr Http2ClientConversation::con()
{
	return con_;
}

} // close namespaces


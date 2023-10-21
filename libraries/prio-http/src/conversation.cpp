#include <event2/event.h>
#include "priohttp/conversation.h"
#include "priocpp/api.h"
#include "priohttp/server/reader.h"
#include "priohttp/server/writer.h"

using namespace repro;

#ifdef _WIN32
#define strcasecmp _stricmp
#endif



namespace prio  {


HttpConversation::HttpConversation(Connection::Ptr f)
	: req(this),
	  res(this),
	  con_(f),
	  keep_alive_(false),
	  flusheaders_func_( [](Request&,Response&)
	  {
		  auto p = promise<>();
		  nextTick([p]()
		  {
			  p.resolve();
		  });
		  return p.future();
	  }),
	  completion_func_( [](Request&,Response&){})
{

	reader_.reset(new HttpHeaderReader(this));
	writer_.reset(new HttpPlainBodyWriter(this));

	REPRO_MONITOR_INCR(HttpConversation);
}
 
HttpConversation::~HttpConversation()
{
	REPRO_MONITOR_DECR(HttpConversation);
}

prio::Callback<Request&,Response&>&  HttpConversation::on(Connection::Ptr s)
{
	auto r =  Ptr( new HttpConversation(s));
	r->self_ = r;

	r->reader_->consume("");

	return r->cb_;
}


Future<std::string> HttpConversation::read()
{
	auto p = repro::promise<std::string>();

	con_->read()
	.then( [p] (Connection::Ptr /*c*/, std::string s)
	{
		p.resolve(s);
	})
	.otherwise([p](const std::exception_ptr& ex)
	{
		p.reject(ex);
	});

	return p.future();
}

Future<> HttpConversation::write(const std::string& s)
{
	auto p = repro::promise<>();

	con_->write(s)
	.then( [p] (Connection::Ptr /*c*/ )
	{
		p.resolve();
	})
	.otherwise([p](const std::exception_ptr& ex)
	{
		p.reject(ex);
	});

	return p.future();
}


void HttpConversation::resolve(Request& req, Response& res)
{ 
    cb_.resolve(req,res);
}

repro::Future<> HttpConversation::flush(Response& res)
{
	auto p = repro::promise();

	flusheaders_func_(req,res)
	.then( [this,&res]()
	{
		if(res.isGzipped() && !res.isChunked())
		{
			writer_.reset( new HttpGzippedBodyWriter(new HttpPlainBodyWriter(this)));
		}
		return writer_->flush();
	})
	.then([p]()
	{
		p.resolve();
	});

	return p.future();
}

void HttpConversation::chunk(const std::string& ch)
{
	HttpResponse& response = (HttpResponse&)res;
	if( !response.headersSent() )
	{
		HttpWriter* w = new HttpChunkedBodyWriter(this);
		if(res.isGzipped())
		{
			w = new HttpGzippedBodyWriter(w);
		}
		writer_.reset(w);
	}

	writer_->write(ch);
}


void HttpConversation::onHeadersComplete(const std::string& b)
{
	bool readBody = req.size() > 0; // TODO || req.isCHunked();

	if(readBody)
	{
		//ServerHttpReader* reader = nullptr;
		/*
		if(!con_->isChunked_)
		{
			reader = new HttpClientContentLengthBodyReader(con_,con_->res.size());
		}
		else
		{
			*/

		std::string s = b;
		reader_.reset(new HttpContentLengthBodyReader(this));

		if (s.empty())
		{
			std::string expect = req.headers.get("Expect");
			if (strcasecmp(expect.c_str(), "100-continue") == 0)
			{
				write("HTTP/1.1 100 Continue\r\n\r\n")
				.then([this]()
				{
					reader_->consume("");
				});
				return;
			}
		}
		reader_->consume(s);
	}
	else
	{
		onRequestComplete(b);
	}
}

void HttpConversation::onRequestComplete(const std::string& b)
{
	HttpRequest& request = (HttpRequest&)req;

	request.body(b);

	keep_alive_ = req.keep_alive();

	cb_.resolve(req,res);
}

void HttpConversation::onResponseComplete(const std::string&)
{
	if(completion_func_)
	{
		completion_func_(req,res);
	}

	HttpRequest& request = (HttpRequest&)req;
	HttpResponse& response = (HttpResponse&)res;

	request.reset();
	response.reset();

    if ( keep_alive_ && !req.detached())
    {
    	reader_.reset(new HttpHeaderReader(this));
		writer_.reset(new HttpPlainBodyWriter(this));

    	reader_->consume("");
        return;
    }

	if(!req.detached())
	{
		con_->close();
		self_.reset();
	}
	else
	{
		self_.reset();
	}
}

void HttpConversation::onRequestError(const std::exception_ptr& ex)
{
	cb_.reject(ex);
	con_->close();
	self_.reset();
}

Connection::Ptr HttpConversation::con()
{
	return con_;
}

void HttpConversation::onCompletion(std::function<void(Request& req, Response& res)> f, Response& /*res*/ )
{
	completion_func_ = f;
}

void HttpConversation::onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& /*res*/ )
{
	flusheaders_func_ = f;
}

bool HttpConversation::keepAlive()
{
	return keep_alive_;
}

std::string HttpConversation::common_name()
{
	return con_->common_name();
}


////////////////


SubRequest::SubRequest()
	: req(this),
	  res(this),
	  completion_func_( [](Request&,Response&){})
{
}
 
SubRequest::~SubRequest()
{
}

prio::Callback<Request&,Response&>& SubRequest::on(const Request& request, const std::string& path)
{
	req.path = request.path;
	req.headers = request.headers;
	req.path.path(path);
	self_ = shared_from_this();

	return cb_;
}

std::string SubRequest::common_name()
{
	return "";
}

void SubRequest::resolve(Request& req, Response& res)
{ 
    cb_.resolve(req,res);
}

repro::Future<> SubRequest::flush(Response& /*res*/ )
{
	auto p = repro::promise();

	nextTick([this,p]()
	{ 
		resolve(this->req,this->res);
		completion_func_(this->req,this->res);
		self_.reset();
		p.resolve(); 
	});

	return p.future();
}

void SubRequest::chunk(const std::string& ch)
{
	res.body(res.body()+ ch);
}

void SubRequest::onRequestError(const std::exception_ptr& ex)
{
	cb_.reject(ex);
	self_.reset();
}

Connection::Ptr SubRequest::con()
{
	Connection::Ptr empty;
	return empty;
}

void SubRequest::onCompletion(std::function<void(Request& req, Response& res)> f, Response& /*res*/ )
{
	completion_func_ = f;
}


void SubRequest::onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& /*res*/ )
{
	flusheaders_func_ = f;
}

bool SubRequest::keepAlive()
{
	return false;
}


} // close namespaces


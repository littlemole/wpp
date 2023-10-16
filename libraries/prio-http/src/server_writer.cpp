#include <event2/event.h>
#include "priohttp/conversation.h"
#include "priocpp/api.h"
#include "priohttp/server/writer.h"

namespace prio  {


repro::Future<> HttpPlainBodyWriter::flush()
{
	auto p = repro::promise();

	HttpResponse& response = (HttpResponse&)(con_->response());

	response.flushHeaders();

	std::string payload = con_->response().toString();

	nextTick()
	.then([this,payload]()
	{
		return con_->write(payload);
	})
	.then( [this,p]()
	{
		con_->onResponseComplete("");
		p.resolve();
	})
	.otherwise( [this,p](const std::exception_ptr& ex)
	{
		con_->onRequestError(ex);
		p.resolve();
	});

	return p.future();
}

void HttpPlainBodyWriter::write(const std::string& c)
{
}



HttpChunkedBodyWriter::HttpChunkedBodyWriter(ReaderWriterConversation* c)
	: HttpBodyWriter(c), writing_(false), done_(false)
{
}


repro::Future<> HttpChunkedBodyWriter::flush()
{
	auto p = repro::promise();

	done_ = true;

	chunks_.push_back("0\r\n\r\n");
	chunkResponse();

	nextTick([p]()
	{
		p.resolve();
	});

	return p.future();
}

void HttpChunkedBodyWriter::write(const std::string& c)
{
	std::ostringstream tmp;
	tmp << std::hex << (int)c.size();

	std::ostringstream oss;
	oss << tmp.str();
	oss << "\r\n";
	oss << c;
	oss << "\r\n";

	HttpResponse& response = (HttpResponse&)(con_->response());

	if(!response.headersSent())
	{
		response.flushHeaders();
		std::ostringstream headers;
		headers << response.status() << "\r\n";
		headers << response.headers.toString();

		chunks_.push_back(headers.str());
	}

	if(c.empty())
	{
		return;
	}

	chunks_.push_back(oss.str());
	chunkResponse();
}

void HttpChunkedBodyWriter::chunkResponse()
{
	if(writing_)
	{
		return;
	}

	if ( chunks_.empty())
	{
		if(done_)
		{
			con_->onResponseComplete("");
			return;
		}

		return;
	}

	writing_ = true;

	std::string chunk = chunks_.front();
	chunks_.pop_front();

	con_->write(chunk)	
	.then( [this]()
	{
		writing_ = false;

		chunkResponse();
		return;
	})
	.otherwise( [this](const std::exception_ptr& ex)
	{
		con_->onRequestError(ex);
	});
}

repro::Future<> HttpGzippedBodyWriter::flush()
{
	HttpBodyWriter* w = (HttpBodyWriter*)wrapped_.get();
	HttpResponse& response = (HttpResponse&)(w->res());

	response.flushHeaders();
	if ( w->res().isChunked())
	{
		std::string s = compressor_.flush();
		w->write(s);
	}
	else
	{
		std::ostringstream oss;
		oss << compressor_.compress(w->res().body());
		oss << compressor_.flush();
		w->res().body(oss.str());
	}
	
	return w->flush();	
}

void HttpGzippedBodyWriter::write(const std::string& c)
{
	std::string s = compressor_.compress(c);
	return wrapped_->write(s);
}


} // close namespaces


#ifndef DEFINE_MOL_HTTP_SERVER_REQUEST_WRITER_PROCESSOR_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_REQUEST_WRITER_PROCESSOR_DEF_GUARD_DEFINE_

#include "priohttp/response.h"

namespace prio  {


class Conversation;
class ServerHttpReader;
class HttpBodyWriter;

class HttpWriter
{
public:

	virtual ~HttpWriter() {}

	virtual repro::Future<> flush() = 0;
	virtual void write(const std::string& c) = 0;

};

class HttpBodyWriter : public HttpWriter
{
public:

	HttpBodyWriter(ReaderWriterConversation* c)
		: con_(c)
	{}

	virtual ~HttpBodyWriter() {}

	virtual repro::Future<> flush() = 0;
	virtual void write(const std::string& c) = 0;

	Request& req() { return con_->request();   }
	Response& res() { return con_->response();   }

protected:

	void write();

	ReaderWriterConversation* con_;
};

class HttpPlainBodyWriter : public HttpBodyWriter
{
public:

	HttpPlainBodyWriter(ReaderWriterConversation* c)
		: HttpBodyWriter(c)
	{}

	virtual ~HttpPlainBodyWriter() {}

	virtual repro::Future<> flush();
	virtual void write(const std::string& c);

};

class HttpChunkedBodyWriter : public HttpBodyWriter
{
public:

	HttpChunkedBodyWriter(ReaderWriterConversation* c);

	virtual ~HttpChunkedBodyWriter() {}

	virtual repro::Future<> flush();
	virtual void write(const std::string& c);

private:

	void chunkResponse();

	bool writing_;
	bool done_;
    std::deque<std::string> chunks_;
};

class HttpGzippedBodyWriter : public HttpWriter
{
public:

	HttpGzippedBodyWriter(HttpWriter* wrapped)
		: wrapped_(wrapped)
	{}

	virtual ~HttpGzippedBodyWriter() {}

	virtual repro::Future<> flush();
	virtual void write(const std::string& c);

private:

	std::unique_ptr<HttpWriter> wrapped_;
	Compressor compressor_;
};



} // close namespaces


#endif




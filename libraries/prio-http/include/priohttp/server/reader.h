#ifndef DEFINE_MOL_HTTP_SERVER_REQUEST_READER_PROCESSOR_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_REQUEST_READER_PROCESSOR_DEF_GUARD_DEFINE_

#include "priohttp/response.h"
 
namespace prio {

class Conversation;
class ServerHttpReader;
class HttpBodyWriter;

class ServerHttpReader
{
public:

	ServerHttpReader(ReaderWriterConversation* c);

	virtual ~ServerHttpReader() {}

	virtual void consume(const std::string& s) = 0;

protected:

	void read();

	ReaderWriterConversation* con_;
};


class HttpHeaderReader : public ServerHttpReader
{
public:
	HttpHeaderReader(ReaderWriterConversation* c);

	virtual ~HttpHeaderReader();

	virtual void consume(const std::string& s);

private:

	bool parse_headers();

	std::string headers_stream_;
	std::string body_stream_;

};

class HttpContentLengthBodyReader : public ServerHttpReader
{
public:

	HttpContentLengthBodyReader(ReaderWriterConversation* c);

	virtual ~HttpContentLengthBodyReader()
	{}

	virtual void consume(const std::string& c);

private:
	size_t size_;
	std::string body_;
};




} // close namespaces


#endif




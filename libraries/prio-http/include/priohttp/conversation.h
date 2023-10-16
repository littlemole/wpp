#ifndef DEFINE_MOL_HTTP_SERVER_REQUEST_PROCESSOR_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_REQUEST_PROCESSOR_DEF_GUARD_DEFINE_

#include "priohttp/response.h"

namespace prio  {

// forwards;
class Loop;
Loop& theLoop();


class ServerHttpReader;
class HttpWriter;


//////////////////////////////////////////////////////////////

class Conversation
{
public:

	virtual ~Conversation() {}

	virtual bool keepAlive() = 0;
	virtual repro::Future<> flush(Response& res) = 0;
	virtual void onCompletion(std::function<void(Request& req, Response& res)> f, Response& res) = 0;
    virtual void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& res) = 0;
	virtual void chunk(const std::string& ch) = 0;
	virtual prio::Connection::Ptr con() = 0;
	virtual void onRequestError(const std::exception_ptr& s)  = 0;
	virtual void resolve(Request& req, Response& res) = 0;
	virtual std::string common_name() = 0;
};
 
class ReaderWriterConversation 
{
public:

	virtual ~ReaderWriterConversation() {}

	virtual void onRequestError(const std::exception_ptr& s)  = 0;
	virtual void onHeadersComplete(const std::string& s)  = 0;
	virtual void onRequestComplete(const std::string& s)  = 0;
	virtual void onResponseComplete(const std::string& s) = 0;

	virtual repro::Future<std::string> read() = 0;
	virtual repro::Future<> write(const std::string& s) = 0;

	virtual Request& request() = 0;
	virtual Response& response() = 0;
};

class HttpConversation : public Conversation, public ReaderWriterConversation, public std::enable_shared_from_this<HttpConversation>
{
public:

	typedef std::shared_ptr<HttpConversation> Ptr;

	Request req;
	Response res;

	static prio::Callback<Request&,Response&>& on(Connection::Ptr client);

    ~HttpConversation();

	virtual repro::Future<> flush(Response& res);
    virtual void onCompletion(std::function<void(Request& req, Response& res)> f, Response& res);
    virtual void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& res);
	virtual void chunk(const std::string& ch);
	virtual bool keepAlive();
	virtual Connection::Ptr con();

	virtual void onHeadersComplete(const std::string& s);
	virtual void onRequestComplete(const std::string& s);
	virtual void onResponseComplete(const std::string& s);
	virtual void onRequestError(const std::exception_ptr& s);

	virtual Request& request() { return req; }
	virtual Response& response() { return res; }
	virtual repro::Future<std::string> read();
	virtual repro::Future<> write(const std::string& s);
	virtual void resolve(Request& req, Response& res);

	virtual std::string common_name() ;

private:

	HttpConversation(Connection::Ptr f);

	HttpConversation(const HttpConversation& rhs) = delete;
	HttpConversation(HttpConversation&& rhs) = delete;
	HttpConversation& operator=(const HttpConversation& rhs) = delete;
	HttpConversation& operator=(HttpConversation&& rhs) = delete;

	Connection::Ptr con_;
	prio::Callback<Request&,Response&> cb_;

	bool keep_alive_;

	std::unique_ptr<ServerHttpReader> reader_;
	std::unique_ptr<HttpWriter> writer_;

	std::function<repro::Future<>(Request& req, Response& res)> flusheaders_func_;
	std::function<void(Request& req, Response& res)> completion_func_;

	Ptr self_;
};

class SubRequest : public Conversation, public std::enable_shared_from_this<SubRequest>
{
public:

	typedef std::shared_ptr<SubRequest> Ptr;

	Request req;
	Response res;

	prio::Callback<Request&,Response&>& on(const Request& request, const std::string& path);

	SubRequest();
    ~SubRequest();

	virtual repro::Future<> flush(Response& res);
    virtual void onCompletion(std::function<void(Request& req, Response& res)> f, Response& res);
    virtual void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& res);
	virtual void chunk(const std::string& ch);
	virtual bool keepAlive();
	virtual Connection::Ptr con();

	virtual void onRequestError(const std::exception_ptr& s);
	virtual void resolve(Request& req, Response& res);
	
	virtual std::string common_name() ;
private:

	SubRequest(const SubRequest& rhs) = delete;
	SubRequest(SubRequest&& rhs) = delete;
	SubRequest& operator=(const SubRequest& rhs) = delete;
	SubRequest& operator=(SubRequest&& rhs) = delete;

	prio::Callback<Request&,Response&> cb_;

	std::function<repro::Future<>(Request& req, Response& res)> flusheaders_func_;
	std::function<void(Request& req, Response& res)> completion_func_;

	Ptr self_;
};

//////////////////////////////////////////////////////////////

} // close namespaces


#endif




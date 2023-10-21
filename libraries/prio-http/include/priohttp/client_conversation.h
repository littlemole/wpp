#ifndef DEFINE_MOL_HTTP_SERVER_RESPONSE_PROCESSOR_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_RESPONSE_PROCESSOR_DEF_GUARD_DEFINE_

#include "priohttp/conversation.h"

namespace prio      {


//////////////////////////////////////////////////////////////

class ClientHttpReader;
class http2_client_session;

class HttpClientConversation : public ReaderWriterConversation, public std::enable_shared_from_this<HttpClientConversation>
{
public:

	typedef std::shared_ptr<HttpClientConversation> Ptr;

	Request req;
	Response res;

	static prio::Callback<Request&,Response&>& on(Connection::Ptr client, Request& r);

    ~HttpClientConversation();

    HttpClientConversation(const HttpClientConversation& rhs) = delete;
    HttpClientConversation& operator=(const HttpClientConversation& rhs) = delete;

	virtual Connection::Ptr con();

	virtual void onHeadersComplete(const std::string& s);
	virtual void onRequestComplete(const std::string& s);
	virtual void onResponseComplete(const std::string& s);
	virtual void onRequestError(const std::exception_ptr& s);
	virtual Request& request() { return req; }
	virtual Response& response() { return res; }

	virtual repro::Future<std::string> read();
	virtual repro::Future<> write(const std::string& s);

	virtual repro::Future<> flush(Response& /*res*/ ) 
	{
		auto p = repro::promise();
		nextTick([p](){ p.resolve(); });
		return p.future();
	}
	virtual void onCompletion(std::function<void(Request& req, Response& res)> /*f*/, Response& /*res*/ ) {};
	virtual void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> /*f*/, Response& /*res*/ ) {};
	virtual void chunk(const std::string& /*ch*/) {};
	
	virtual bool keepAlive()
	{
		return keep_alive_;
	}

	virtual std::string common_name();
	
private:

	prio::Callback<Request&,Response&> cb_;

	HttpClientConversation(Connection::Ptr client, Request& r);

	void read_channel();

	Connection::Ptr con_;
	std::function<void(const std::string& s)> status_func_;


	bool keep_alive_;
	Ptr self_;

	std::unique_ptr<ClientHttpReader> reader_;
};

//////////////////////////////////////////////////////////////

class Http2ClientConversation : public Conversation, public std::enable_shared_from_this<Http2ClientConversation>
{
public:

	typedef std::shared_ptr<Http2ClientConversation> Ptr;

	Request req;

	static prio::Callback<Request&,Response&>& on(Connection::Ptr client, Request& r);

    ~Http2ClientConversation();

    Http2ClientConversation(const Http2ClientConversation& rhs) = delete;
    Http2ClientConversation& operator=(const Http2ClientConversation& rhs) = delete;

	virtual Connection::Ptr con();

	virtual void onRequestError(const std::exception_ptr& s);
	virtual repro::Future<> flush(Response& /*res*/ ) 
	{
		auto p = repro::promise();
		nextTick([p](){ p.resolve(); });
		return p.future();
	}

	virtual void onCompletion(std::function<void(Request& req, Response& res)> /*f*/, Response& /*res*/ ) {};
	virtual void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> /*f*/, Response& /*res*/) {};

	virtual void chunk(const std::string& /*ch*/ ) {};
	
	virtual bool keepAlive()
	{
		return keep_alive_;
	}

	void resolve(Request& req, Response& res);

	virtual std::string common_name();

private:

	prio::Callback<Request&,Response&> cb_;

	Http2ClientConversation(Connection::Ptr client, Request& r);

	void schedule_read();

	Connection::Ptr con_;
	std::function<void(const std::string& s)> status_func_;

	bool keep_alive_;
	Ptr self_;

	std::unique_ptr<http2_client_session> http2_;
};


} // close namespaces


#endif




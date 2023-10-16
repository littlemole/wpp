#ifndef DEFINE_MOL_HTTP2_SERVER_REQUEST_PROCESSOR_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP2_SERVER_REQUEST_PROCESSOR_DEF_GUARD_DEFINE_

#include "priohttp/conversation.h"

namespace prio  {

class http2_server_session;
 
class Http2Conversation : public Conversation, public std::enable_shared_from_this<Http2Conversation>
{
friend class http2_session;
public:

	typedef std::shared_ptr<Http2Conversation> Ptr;

	static prio::Callback<Request&,Response&>&  on(Connection::Ptr client);

    ~Http2Conversation();

	virtual repro::Future<> flush(Response& res);
    virtual void onCompletion(std::function<void(Request& req, Response& res)> f, Response& res);
	virtual void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& res);
	virtual void chunk(const std::string& ch);
	virtual void onRequestError(const std::exception_ptr& s);	
	virtual bool keepAlive();
	virtual Connection::Ptr con();
	virtual void resolve(Request& req, Response& res);

	virtual std::string common_name();

private:

    void schedule_read();

	Http2Conversation(Connection::Ptr f);
	
	Http2Conversation(const Http2Conversation& rhs) = delete;
	Http2Conversation(Http2Conversation&& rhs) = delete;
	Http2Conversation& operator=(const Http2Conversation& rhs) = delete;
	Http2Conversation& operator=(Http2Conversation&& rhs) = delete;

	Connection::Ptr con_;
	prio::Callback<Request&,Response&> cb_;

    Ptr self_;
    
    std::unique_ptr<http2_server_session> http2_;
};

//////////////////////////////////////////////////////////////

} // close namespaces


#endif




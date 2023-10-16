#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SSE_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SSE_DEF_GUARD_

//! \file sse.h
//! \defgroup ws

#include <map>
#include <set>

#include <priohttp/request.h>
#include <priohttp/attr.h>

//////////////////////////////////////////////////////////////


namespace reproweb    {

//! SSE Connection
//! \ingroup ws
class SSEConnection : public std::enable_shared_from_this<SSEConnection>
{
public:

	prio::Request req;

	~SSEConnection()
    {}

	typedef std::shared_ptr<SSEConnection> Ptr;

	//! \private
	static Ptr create()
	{
		Ptr ptr = Ptr(new SSEConnection());
		ptr->self_ = ptr;
		return ptr;
	}

	//! \private
    void handshake(prio::Request& r)
    {
		req = r;
        con_ = r.con();

        HttpRequest& request = (HttpRequest&)r;
        request.detach();
        request.con()->timeouts().rw_timeout_s = 1000L * 60L * 10L;
    }

	//! send some text over the SSE connection
    void send( const std::string& s )
    {
		if(!con_)
			return;

        std::ostringstream oss;
        oss << "data: " << s << "\n\n";

        con_->write(oss.str())
        .then([](prio::Connection::Ptr c){})
        .otherwise([this](const std::exception& ex)
        {
            close();
        });
    }

	//! attach SSE onClose handler
    template<class T>
    SSEConnection* onClose( T t )
    {
        onClose_ = t;
        return this;
    }

	//! attach SSE onConnect handler
    template<class T>
    SSEConnection* onConnect( T t )
    {
        onConnect_ = t;
        return this;
    }

	//! \private
    void run()
    {
    	onConnect_(shared_from_this());
    }

	//! close the websocket
    void close()
    {
		onClose_(shared_from_this());
        dispose();
    }

	//! \private
	void dispose()
    {
        if(con_)
        {
            con_->cancel();
            con_->close();
            con_.reset();
        }
        self_.reset();        
    }

	//! \private
	prio::Connection::Ptr connection();

private:

    SSEConnection()
    {
        onConnect_ = [](Ptr){};
        onClose_ = [](Ptr){};        
    }

	std::function<void(Ptr)> onConnect_;
	std::function<void(Ptr)> onClose_;

	Ptr self_;
	prio::Connection::Ptr con_;
};

//! \private
template<class T,class ... Args >
void sse_callback(SSEConnection::Ptr sse, void (T::*fun)(SSEConnection::Ptr, const Args& ... args), const Args& ... args )
{
	try
	{
		auto ptr = sse->req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<T>();
		T* t = ptr.get();
		(t->*fun)(sse,args...);
	}
	catch(...)
	{
		prio::nextTick([sse]()
		{
			sse->close();
		});
	}
};

//! \private
template<class T,class ... Args>
void sse_callback(SSEConnection::Ptr sse, repro::Future<> (T::*fun)(SSEConnection::Ptr, const Args& ... args), const Args& ... args )
{
	try
	{
		auto ptr = sse->req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<T>();
		T* t = ptr.get();
		(t->*fun)(sse,args...)
		.then([]()
		{
			// all good
		})
		.otherwise([sse](const std::exception& ex)
		{
			sse->close();
		});
	}
	catch(...)
	{
		prio::nextTick([sse]()
		{
			sse->close();
		});
	}
};



//! sse controller
//! \ingroup ws

template<class T>
class sse_controller
{
public:

	typedef T type;

	//! register sse_controller for given HTTP path
	sse_controller( const std::string& path)
	  : path_(path)
	{
		//std::cout << "declare WS " << path_ << std::endl;		
	}

	//! \private
	void ctx_register(diy::Context* ctx)
	{
		//std::cout << "register WS " << path_ << std::endl;
		
		auto fc = ctx->resolve<FrontController>();

		fc->registerHandler(
			"GET",
			path_,
			[](prio::Request& req, prio::Response& res)
			{
				handshake(req, res);
			}
		);
	
	}

private:

	static void handshake(prio::Request& req, prio::Response& res)
	{
		reproweb::SSEConnection::Ptr sse = reproweb::SSEConnection::create();

	    sse->handshake(req);

	    sse->onConnect([](SSEConnection::Ptr sse)
	    {
			sse_callback(sse,&T::onConnect);
	    });

	    sse->onClose([](SSEConnection::Ptr sse)
	    {
			sse_callback(sse,&T::onClose);
	    });

	    res
		.contentType("text/event-stream")
		.header("content-length","-")
	    .ok()
	    .flush()
		.then([sse]()
		{
			sse->run();
		});
	};

	std::string path_;
};


} // close namespaces

#endif


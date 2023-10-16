#ifndef INCLUDE_PROMISE_WS_WS_H_
#define INCLUDE_PROMISE_WS_WS_H_

//! \file ws.h

#include "reproweb/ws/websocket.h"
#include <diycpp/ctx.h>

//////////////////////////////////////////////////////////////

namespace reproweb   {

//! \private
template<class T,class ... Args >
void ws_callback(WsConnection::Ptr ws, void (T::*fun)(WsConnection::Ptr, const Args& ... args), const Args& ... args )
{
	try
	{
		auto ptr = ws->req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<T>();
		T* t = ptr.get();
		(t->*fun)(ws,args...);
	}
	catch(...)
	{
		prio::nextTick([ws]()
		{
			ws->close();
		});
	}
};

//! \private
template<class T,class ... Args>
void ws_callback(WsConnection::Ptr ws, repro::Future<> (T::*fun)(WsConnection::Ptr, const Args& ... args), const Args& ... args )
{
	try
	{
		auto ptr = ws->req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<T>();
		T* t = ptr.get();
		(t->*fun)(ws,args...)
		.then([]()
		{
			// all good
		})
		.otherwise([ws](const std::exception& ex)
		{
			ws->close();
		});
	}
	catch(...)
	{
		prio::nextTick([ws]()
		{
			ws->close();
		});
	}
};

//! web-socket controller
//! \ingroup ws

template<class T>
class ws_controller
{
public:

	typedef T type;

	//! register ws_controller for given HTTP path
	ws_controller( const std::string& path)
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
		reproweb::WsConnection::Ptr ws = reproweb::WsConnection::create();

	    std::string hash = ws->handshake(req);

	    if(hash.empty())
	    {
			res.error().flush();
			return;
	    }

	    ws->onConnect([](WsConnection::Ptr ws)
	    {
			ws_callback(ws,&T::onConnect);
	    });

	    ws->onClose([](WsConnection::Ptr ws)
	    {
			ws_callback(ws,&T::onClose);
	    });

	    ws->onMsg([](WsConnection::Ptr ws, const std::string& data)
		{
			ws_callback(ws,&T::onMsg,data);
		});

	    res
		.header("Upgrade","websocket")
	    .header("Connection","Upgrade")
	    .header("Sec-WebSocket-Accept",hash)
	    .status("HTTP/1.1 101 Switching Protocols")
	    .flush()
		.then([ws]()
		{
		    ws->run();
		});
	};

	std::string path_;
};



} // close namespaces

#endif


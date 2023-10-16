#include <event2/event.h>
#include <fcntl.h>
#include "priohttp/conversation.h"
#include "priohttp/http2_conversation.h"
#include "priocpp/api.h"
#include "priohttp/http_server.h"
#include "priocpp/ssl_connection.h"
#include <iostream>

namespace prio  {


http_server::http_server()
 : isSecure_(false)
{
	listener_ = std::make_shared<Listener>();
}


http_server::http_server(prio::SslCtx& ctx)
 : isSecure_(true)
{
	listener_ = std::make_shared<Listener>(ctx);	
}

http_server::~http_server()
{
	listener_->cancel();
}	

void http_server::shutdown()
{
	listener_->cancel();
}

prio::Callback<Request&,Response&>& http_server::bind(int port)
{
	listener_->bind(port)
	.then( [this,port](Connection::Ptr client)
	{
		if(client->isHttp2Requested())
		{
			onAccept2(client,port);			
		}
		else
		{
			onAccept(client,port);			
		}
	})
	.otherwise([](const std::exception& ex){
		std::cerr << "http_server ex: " << ex.what() << std::endl;
	});	
	return cb_;
}
    
void http_server::onAccept(Connection::Ptr client, int port)
{
    try 
    {
		HttpConversation::on(client)
		.then( [this,port] (Request& req, Response& res) 
		{
			req.attributes.set("server_port",port);
			req.attributes.set("is_secure",isSecure_);

			cb_.resolve(req,res);
		})
		.otherwise( [this] (const std::exception_ptr& ex) 
		{
			 cb_.reject(ex);
		});
    }
    catch( repro::Ex& ex)
    {
        std::cerr << "ex: " << ex.msg << std::endl;
    }  
} 

void http_server::onAccept2(Connection::Ptr client, int port)
{
    try 
    {
		Http2Conversation::on(client)
		.then( [this,port] (Request& req, Response& res) 
		{
			req.attributes.set("server_port",port);
			req.attributes.set("is_secure",isSecure_);

			cb_.resolve(req,res);
		})
		.otherwise( [this] (const std::exception_ptr& ex) 
		{
			 cb_.reject(ex);
		});
    }
    catch( repro::Ex& ex)
    {
        std::cerr << "ex: " << ex.msg << std::endl;
    }  
} 


} // close namespaces



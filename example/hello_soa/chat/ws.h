
#ifndef _MOL_DEF_GUARD_DEFINE_CHAT_PROMISE_EXAMPLE_WS_DEF_
#define _MOL_DEF_GUARD_DEFINE_CHAT_PROMISE_EXAMPLE_WS_DEF_

#include "model.h"
#include "service.h"
#include "reproweb/ws/ws.h"
#include "reproweb/json/redisbus.h"

using namespace reproweb;


struct Redis : public reproredis::RedisPool
{
	Redis(std::shared_ptr<AppConfig> config) 
	  : RedisPool(config->getString("redis")) 
	{}
};



class WebSocketController
{
public:

	// c'tor
	WebSocketController(
		std::shared_ptr<SessionService> s, 
		std::shared_ptr<RedisBus> bus
	)
	  : session_(s), 
	  	eventBus_(bus)
	{}

	// websocket connects
    void onConnect(WsConnection::Ptr ws)
    {
		std::cout << "on connect" << std::endl;
		// subscribe to topic and pass any Json downwards on invocation
    	std::string sid = eventBus_->subscribe("chat-topic", [ws] (Json::Value value)
    	{
    		ws->send(0x01,JSON::stringify(value));
    	});

		std::cout << "on connected" << std::endl;

		// make this ws remember the subscriber id
		set_subscriber_id(ws,sid);
    };

	// websocket disconnects
    void onClose(WsConnection::Ptr ws)
    {
		// unsubscribe from topic using supscription id
    	eventBus_->unsubscribe("chat-topic", subscriber_id(ws));
    };

	// receive a websocket msg
    void onMsg(reproweb::WsConnection::Ptr ws, const std::string& data)
	{
		try 
		{
			std::cout << "ws: " << data << std::endl;

			// parse msg
			Json::Value json = JSON::parse(data);
			std::string sid  = json["sid"].asString();
			std::string msg  = json["msg"].asString();

			// validate session			
			session_->get_user_session(sid)
			.then([this,msg](reproweb::Session session)
			{
				if(!session.authenticated)
				{
					throw AuthEx();
				}
				// populate result
				Json::Value profile = session.data;

				Json::Value result(Json::objectValue);
				result["uid"]   = profile["username"];
				result["login"] = profile["login"];
				result["img"]   = profile["avatar_url"];
				result["msg"]   = msg;

				// publish msg to all subscribers
				eventBus_->notify("chat-topic",result);

			})
			.otherwise([ws](const std::exception& )
			{
				ws->close();
			});

		}
		catch(const std::exception& ex)
		{
    		ws->close();
		}
	};

private:

	void set_subscriber_id(WsConnection::Ptr ws, const std::string& id)
	{
		ws->req.attributes.set("subscription-id",id);
	}

	std::string subscriber_id(WsConnection::Ptr ws)
	{
		return ws->req.attributes.attr<std::string>("subscription-id");
	}

    std::shared_ptr<SessionService> session_;
    std::shared_ptr<RedisBus> eventBus_;
};

#endif



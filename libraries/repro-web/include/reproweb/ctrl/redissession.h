#ifndef INCLUDE_PROMISE_WEB_CONTROLLER_REDIS_SESSION_H_
#define INCLUDE_PROMISE_WEB_CONTROLLER_REDIS_SESSION_H_

#include "reproweb/serialization/json.h"
#include "reproweb/ctrl/session.h"
#include "reproredis/redis.h"
#include "priocpp/api.h"

//! \file redissession.h
//! \defgroup session

namespace reproweb  
{

//! Redis backed HTTP session
//! \ingroup session
class RedisSessionProvider : public SessionProvider
{
public:

	//! construct RedisSessionProvider with given Redis connection pool
    RedisSessionProvider( std::shared_ptr<reproredis::RedisPool> r)
        : redis_(r)
    {}

    virtual ~RedisSessionProvider() {}


    virtual repro::Future<Session> get_session( std::string sid)
    {
		auto p = repro::promise<Session>();

        auto session = std::make_shared<Session>(sid,Json::objectValue);

		redis_->cmd("GET", sid) 
		.then([this,p,session](reproredis::RedisResult::Ptr reply)
		{
			if( reply->isError() )
			{
				std::cout << "redis err " << reply->isError() << std::endl;
				throw repro::Ex("invalid session");
			}

			if( !reply->isNill() )
			{
				Json::Value json = JSON::parse(reply->str());
				meta::fromJson(json,*session);
			}

            return redis_->cmd("EXPIRE", session->sid, 60);
		})
		.then([p,session](reproredis::RedisResult::Ptr reply)
		{
			p.resolve( *session );
		})        
		.otherwise([p,session](const std::exception& ex)
		{
			p.resolve(*session);
		});

		return p.future();
    }

    virtual repro::Future<> set_session(Session session)
    {
		auto p = repro::promise<>();

		std::string json = JSON::stringify(meta::toJson(session));

		std::string sid = session.sid;

		redis_->cmd("SET", sid, json)
		.then([this,sid](reproredis::RedisResult::Ptr reply)
		{
			return redis_->cmd("EXPIRE", sid, 60);
		})
		.then([p,session](reproredis::RedisResult::Ptr reply)
		{
			p.resolve();
		})
		.otherwise(repro::reject(p));

		return p.future();
    }


	virtual repro::Future<> remove_user_session(Session session)
	{
		auto p = repro::promise<>();

		redis_->cmd("DEL", session.sid)
		.then([p](reproredis::RedisResult::Ptr reply)
		{
			p.resolve();
		})
		.otherwise(repro::reject(p));

		return p.future();
	}

private:
    std::shared_ptr<reproredis::RedisPool> redis_;
};



}

#endif

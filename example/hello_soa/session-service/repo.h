#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "reproredis/redis.h"
#include "entities.h"

MAKE_REPRO_EX(RedisEx);


struct RedisJsonMapper
{
	std::shared_ptr<reproredis::RedisPool> redis;

	RedisJsonMapper(std::shared_ptr<reproredis::RedisPool> r)
		: redis(r)
	{}

	Future<> put(const std::string& key, Json::Value json, int expire = 0)
	{
		auto p = promise<>();

		redis->cmd("SET", key, json) 
		.then([p,key,expire](reproredis::RedisResult::Ptr reply)
		{
			if(!expire)
			{
				p.resolve();
			}
			else
			{
				reply->cmd("EXPIRE", key, expire)
				.then([p](reproredis::RedisResult::Ptr reply)
				{
					p.resolve();
				})
				.otherwise(reject(p));
			}
		})
		.otherwise(reject(p));

		return p.future();
	}

	Future<Json::Value> get(const std::string& key, int expire = 0)
	{
		auto p = promise<Json::Value>();

		redis->cmd("GET", key)
		.then([p,key,expire](reproredis::RedisResult::Ptr reply)
		{
			if(reply->isError() || reply->isNill())
			{
				throw RedisEx("invalid key");
			}

			std::string payload = reply->str();

			if(!expire)
			{
				Json::Value json = JSON::parse(payload);
				p.resolve(json);				
			}
			else
			{
				reply->cmd("EXPIRE", key, expire)
				.then( [p,payload](reproredis::RedisResult::Ptr reply)
				{
					Json::Value json = JSON::parse(payload);
					p.resolve(json);				
				})
				.otherwise([p](const std::exception& ex)
				{
					p.reject(RedisEx(ex.what()));
				});				
			}			
		})
		.otherwise([p](const std::exception& ex)
		{
			p.reject(RedisEx(ex.what()));
		});		

		return p.future();
	}

	Future<> remove(const std::string& key)
	{
		auto p = promise<>();

		redis->cmd("DEL", key) 
		.then([p](reproredis::RedisResult::Ptr reply)
		{
			p.resolve();
		})
		.otherwise(reject(p));

		return p.future();
	}

};

struct RedisMapper
{
	std::shared_ptr<reproredis::RedisPool> redis;

	RedisMapper(std::shared_ptr<reproredis::RedisPool> r)
		: redis(r)
	{}

	template<class T>
	Future<> put(const std::string& key, T& t, int expire = 0)
	{
		Json::Value json = meta::toJson(t);

		return RedisJsonMapper(redis).put(key,json,expire);
	}	

	template<class T>
	Future<T> get(const std::string& key, int expire = 0)
	{
		auto p = promise<T>();

		RedisJsonMapper(redis).get(key,expire)
		.then( [p](Json::Value json)
		{
			T t;
			meta::fromJson(json,t);
			p.resolve(t);
		})
		.otherwise(reject(p));

		return p.future();
	}

	Future<> remove(const std::string& key)
	{
		return RedisJsonMapper(redis).remove(key);
	}	

};

class SessionRepository
{
public:

	SessionRepository(std::shared_ptr<reproredis::RedisPool> redisPool)
		: redis(redisPool)
	{}

	Future<Session> get_user_session( std::string sid)
	{
		return RedisMapper(redis).get<Session>(sid,180);
		/*
		auto p = promise<Session>();

		auto payload = std::make_shared<std::string>();

		redis->cmd("GET", sid)
		.then([this,sid,payload](reproredis::RedisResult::Ptr reply)
		{
			if(reply->isError() || reply->isNill())
			{
				throw NoSessionEx("invalid session");
			}

			*payload = reply->str();

			return redis->cmd("EXPIRE", sid, 180);
		})
		.then([p,sid,payload](reproredis::RedisResult::Ptr reply)
		{
			Json::Value json = reproweb::JSON::parse(*payload);
			Session session;
			fromJson(json,session);
			p.resolve(session);
		})
		.otherwise(reject(p));

		return p.future();
		*/
	}

	Future<Session> write_user_session(Session session)
	{
		auto p = promise<Session>();

		RedisMapper(redis).put(session.sid,session,180)
		.then([p,session]()
		{
			p.resolve(session);
		})
		.otherwise(reject(p));
/*
		redis->cmd("SET", session.sid, toJson(session) )
		.then([this,session](reproredis::RedisResult::Ptr reply)
		{
			return redis->cmd("EXPIRE", session.sid, 180);
		})
		.then([p,session](reproredis::RedisResult::Ptr reply)
		{
			p.resolve(session);
		})
		.otherwise(reject(p));
*/
		return p.future();
	}

	Future<> remove_user_session( std::string sid)
	{
		return RedisMapper(redis).remove(sid);
/*
		auto p = promise<>();

		redis->cmd("DEL", sid)
		.then([p](reproredis::RedisResult::Ptr reply)
		{
			p.resolve();
		})
		.otherwise(reject(p));

		return p.future();
*/		
	}

private:

	std::shared_ptr<reproredis::RedisPool> redis;
};


 

struct SessionPool : public reproredis::RedisPool
{
	SessionPool(std::shared_ptr<Config> config) 
	  : RedisPool(config->getString("redis")) 
	{}
};


#endif

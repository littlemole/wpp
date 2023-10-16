
#ifndef _MOL_DEF_GUARD_DEFINE_REDISBUS_DEF_
#define _MOL_DEF_GUARD_DEFINE_REDISBUS_DEF_

#include "reproweb/json/json.h"
#include "reproredis/redis.h"

using namespace reproweb;


class RedisBus
{
public:

	RedisBus(std::shared_ptr<reproredis::RedisPool> p)
		: redis_(p)
	{}

	//! subscribe a callback to a topic, return subscrption id
	std::string subscribe( const std::string topic, std::function<void(Json::Value)> observer)
	{
		std::cout << "ws subscribe " << topic << std::endl;
		// if there is no redis subscriber for this topic yet, create one
		if(listeners_.count(topic) == 0)
		{
			redis_subscribe(topic,observer);
		}

		std::string id = cryptoneat::nonce(32);
		prio::nextTick( [this,id,topic,observer] ()
		{
			subscriptions_[topic].insert(std::make_pair(id, subscription(observer)));
		});
		return id;		
	}

	//! unsubscribe with subscription id
	void unsubscribe( const std::string& topic,  const std::string& id)
	{
		prio::nextTick( [this,topic,id] ()
		{
			subscriptions_[topic].erase(id);
			
			if(subscriptions_[topic].size() == 0)
			{
				listeners_[topic]->unsubscribe();
				listeners_.erase(topic);
			}
			
		});
	}

	//! raise an Event to a topic using Redis passing JSON as payload
	void notify(const std::string topic, Json::Value value)
	{
		std::string msg = JSON::flatten(value);

		std::cout << "send: " << topic << ": " << msg << std::endl;

		redis_->cmd("publish", topic, msg)
		.then([](reproredis::RedisResult::Ptr r)
		{
			std::cout << "MSG SEND! " << r->str() << std::endl;
		})			
		.otherwise([](const std::exception& ex)
		{
			std::cout << "redis send failed: " <<  ex.what() << std::endl;
		});		
	}

	//! clear all subscriptions
	void clear()
	{
		subscriptions_.clear();
		listeners_.clear();
	}

private:

	//! subscribe a callback to a topic, return subscrption id
	void redis_subscribe( const std::string topic, std::function<void(Json::Value)> observer)
	{
		std::cout << "redis resubscribe " << topic << std::endl;

		reproredis::RedisSubscriber* sub = new reproredis::RedisSubscriber(*(redis_.get()));
		sub->subscribe(topic)
		.then( [this] (std::pair<std::string,std::string> msg)
		{
			std::cout  << "msg: " << msg.first << ": " << msg.second << std::endl;
			this->notified(msg.first,msg.second);
		})			
		.otherwise([this,topic,observer](const std::exception& ex)
		{
			std::cout << "!" << ex.what() << std::endl;
			timeout([this,topic,observer]()
			{
				redis_subscribe(topic,observer);
			},10);
		});				

		listeners_[topic] = std::unique_ptr<reproredis::RedisSubscriber>(sub);
	}

	void notified(std::string topic, std::string msg)
	{
		for( auto fun : subscriptions_[topic])
		{
			prio::nextTick( [fun,msg]()
			{
				fun.second.fun(JSON::parse(msg));
			});
		}
	}

	std::shared_ptr<reproredis::RedisPool> redis_;

	std::map<std::string,std::map<std::string,subscription>> subscriptions_;
	std::map<std::string,std::unique_ptr<reproredis::RedisSubscriber>> listeners_;
};

#endif

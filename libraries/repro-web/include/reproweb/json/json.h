#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_RESPONSE_JSON_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_RESPONSE_JSON_DEF_GUARD_

//! \file json.h
//! \defgroup json

#include <json/json.h>
#include <reprocpp/ex.h>
#include <priocpp/api.h>
#include <reprocpp/promise.h>
#include <reproweb/traits.h>
#include <metacpp/json.h>
#include <algorithm>

namespace reproweb  {


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

//! EventBus subscription
//! \ingroup json

struct subscription
{
	 //! construct subscription with given callback handler
	 subscription( std::function<void(Json::Value)> f) : fun(f) {}

	 //! the sunscription callback
	 std::function<void(Json::Value)> fun;
};

//! \private
inline bool operator<(const subscription& lhs, const subscription& rhs)
{
	 if ( &lhs == &rhs)
	 {
		 return false;
	 }

	 return &lhs.fun < &rhs.fun;
}

//! JSON asyncrhonous EventBus
//! \ingroup json

class EventBus
{
public:

	//! subscribe a callback to a topic, return subscrption id
	std::string subscribe( const std::string& topic, std::function<void(Json::Value)> observer);

	//! unsubscribe with subscription id
	void unsubscribe( const std::string& topic,  const std::string& id);

	//! raise an Event to a topic using Eventbus passing JSON as payload
	void notify(const std::string& topic, Json::Value value);

	//! clear all subscriptions
	void clear();

private:

	std::map<std::string,std::map<std::string,subscription>> subscriptions_;
};


//! \private
template<class F>
repro::Future<> forEach( std::shared_ptr<Json::Value> json, unsigned int index, F f )
{
	auto p = repro::promise<>();

	if ( index == json->size() )
	{
		prio::nextTick([p]()
		{
			p.resolve();
		});
		
		return p.future();
	}

	int step = index;
	step++;

	f((*json)[index])
	.then([json,step,f]()
	{
		return forEach(json,step,f);
	})
	.then([p]()
	{
		p.resolve();
	})
	.otherwise(repro::reject(p));

	return p.future();
}

//! call a callback returning Future<> for each element of JSON Array
//! \ingroup json

template<class F>
repro::Future<> forEach(Json::Value json, F f )
{
	auto container = std::make_shared<Json::Value>(json);
	int i = 0;

	return forEach( container, i, f);
}

//! sort JSON Array by sorting all Objects given a specific member value (numeric)
//! \ingroup json
inline Json::Value sort(const Json::Value& arrayIn, const std::string member)
{
	std::vector<Json::Value> v;

	for( unsigned int i = 0; i < arrayIn.size(); i++)
	{
		v.push_back(arrayIn[i]);
	}

	std::sort( 
		v.begin(), 
		v.end(), 
		[member]( const Json::Value& lhs, const Json::Value& rhs ) 
		{
			return lhs[member].asInt() < rhs[member].asInt();
		}
	);

	Json::Value result = Json::Value(Json::arrayValue);
	for ( auto& item : v ) 
	{
		result.append(item);
	}
	return result;
}

}

#endif


#ifndef _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_JSON_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_JSON_DEF_GUARD_

#include <reproweb/json/json.h>
#include <reproweb/serialization/parameter.h>
#include <metacpp/json.h>
#include "reproweb/ctrl/front_controller.h"

namespace reproweb {


///////////////////////////////////////////////////////////////////////////////////////////

template<class T>
struct json_t
{
	T value;

	T* operator->()
	{
		return &value;
	}

	T& operator*()
	{
		return value;
	}
};

template<class T>
using async_json_t = repro::Future<json_t<T>>;

template<class T>
auto json_promise()
{
	return repro::promise<json_t<T>>();
}

///////////////////////////////////////////////////////////////////////////////////////////


template<class T>
class HandlerParam<json_t<T>>
{
public:

	static json_t<T> get(prio::Request& req,  prio::Response& res)
	{	
		Json::Value json = JSON::parse(req.body());

		json_t<T> t;
		meta::fromJson(json,t.value);

		std::cout << JSON::stringify(meta::toJson(t.value)) << std::endl;
		validate(t.value);

		return t;
	}
};

//////////////////////////////////////////////////////////////

template<>
class HandlerParam<Json::Value>
{
public:

	static Json::Value get(prio::Request& req,  prio::Response& res)
	{
		Json::Value json = JSON::parse(req.body());

		return json;
	}
};


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

inline void output_json(prio::Response& res,Json::Value json)
{
	res
	.body(JSON::flatten(json))
	.contentType("application/json")
	.ok()
	.flush();
}

template<class T>
void output_json(prio::Response& res, T& t)
{
	output_json(res, meta::toJson(t) );
}


template<class T>
void output_json(prio::Response& res, json_t<T>& t)
{
	output_json(res, meta::toJson(t.value) );
}

//////////////////////////////////////////////////////////////


template<class R,class C, class ... Args>
Async invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, repro::Future<json_t<R>> (C::*fun)(Args...) )
{
	try
	{
		std::cout << req.body() << std::endl;
		C& c = prepare_controller<C>(req);
		json_t<R> r = co_await (c.*fun)(HandlerParam<Args>::get(req,res)...);		
		output_json(res,r);
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}

	(void)(co_await prio::nextTick());
	co_return;
}




template<class C, class ... Args>
Async invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, repro::Future<Json::Value> (C::*fun)(Args...) )
{
	try
	{
		C& c = prepare_controller<C>(req);
		Json::Value r = co_await (c.*fun)(HandlerParam<Args>::get(req,res)...);	
		output_json(res,r);		
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}

	(void)(co_await prio::nextTick());
	co_return;
}


///////////////////////////////////////////////////////////////////////////////////////////

}

#endif


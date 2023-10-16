#ifndef INCLUDE_PROMISE_WEB_CONTROLLER_XXX_PARAMETER_H_
#define INCLUDE_PROMISE_WEB_CONTROLLER_XXX_PARAMETER_H_

#include "priohttp/multipart.h"
#include "priohttp/response.h"
#include "metacpp/meta.h"
#include "reproweb/traits.h"
#include "reproweb/ctrl/front_controller.h"

namespace reproweb  {

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class C>
C& prepare_controller(prio::Request& req)
{
	auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
	req.attributes.set("controller", ptr);
	C& c = *ptr;		
	return c;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template <typename T>
void validate( T& t)
{
     call_valid::invoke(t);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////

template<class T>
struct Form
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

//////////////////////////////////////////////////////////////

template<class T>
struct Parameter
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

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class T,class E = void>
class HandlerParam;

//////////////////////////////////////////////////////////////


template<class C>
void invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, void (C::*fun)(prio::Request&, prio::Response&) )
{
	try 
	{	
		C& c = prepare_controller<C>(req);
		(c.*fun)(req,res);
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}
}


template<class C, class ... Args>
void invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, void (C::*fun)(Args...) )
{
	try
	{		
		C& c = prepare_controller<C>(req);
		(c.*fun)(HandlerParam<Args>::get(req,res)...);		
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}
}

//////////////////////////////////////////////////////////////


template<class C, class ... Args>
Async invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, Async (C::*fun)(Args...) )
{
	try
	{
		C& c = prepare_controller<C>(req);
		(void)(co_await (c.*fun)(HandlerParam<Args>::get(req,res)...));
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}

	(void)(co_await prio::nextTick());
	co_return;
}


//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////

template<class C, class ... Args>
Async invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, repro::Future<std::string> (C::*fun)(Args...) )
{
	try
	{
		C& c = prepare_controller<C>(req);
		std::string r = co_await (c.*fun)(HandlerParam<Args>::get(req,res)...);		

		if(res.headers.content_type().empty())
		{
			res.contentType("text/html");
		}
		res.body(r).ok().flush();
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}

	(void)(co_await prio::nextTick());
	co_return;
}
 

//////////////////////////////////////////////////////////////


}

#endif

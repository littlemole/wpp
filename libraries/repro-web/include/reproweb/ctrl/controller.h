#ifndef INCLUDE_PROMISE_WEB_CONTROLLER_H_
#define INCLUDE_PROMISE_WEB_CONTROLLER_H_

//! \file controller.h
//! \defgroup controller

#include "reproweb/serialization/parameter.h"
#include <diycpp/injector.h>
#include <diycpp/ctx.h>


namespace reproweb  {

//////////////////////////////////////////////////////////////	
//////////////////////////////////////////////////////////////

template<class F>
class router
{
public:

	typedef F type;

	router(const std::string& m, const std::string& p, F f  )
		: method(m), path(p), handler(f)
	{}


	std::string method;
	std::string path;
	F handler;

	void ctx_register(diy::Context* ctx)
	{
		auto fc = ctx->resolve<FrontController>();
		registerController(*fc,method,path,handler);
	}

private:

	template<class C>
	std::string http_path(std::string p)
	{
		std::ostringstream oss;
		oss << http_root<C>() << p;
		return oss.str();
	}

	template<class C,class ... Args>
	void registerController(FrontController& fc, const std::string& m, const std::string& p, void (C::*fun)(Args...) )
	{
		std::string path = http_path<C>(p);

		fc.registerHandler(m,path, [&fc,fun]( prio::Request& req,  prio::Response& res)
		{			
			invoke_handler(fc,req,res,fun);
		});
	}

	template<class C, class R, class ... Args>
	void registerController(FrontController& fc, const std::string& m, const std::string& p, R(C::*fun)(Args...))
	{
		std::string path = http_path<C>(p);

		fc.registerHandler(m, path, [fun,&fc](prio::Request& req, prio::Response& res)
		{
			invoke_handler(fc,req,res,fun)
			.then([](){})
			.otherwise([](const std::exception& ex)
			{ 
				std::cout << ex.what() << std::endl;
			});
		});
	}

};


//! register HTTP route
//! \ingroup controller
//!
//! \param m - the HTTP method ie GET,POST etc
//! \param p - the HTTP request url path, ie /myapp/index.html
//! \param fun - the HTTP controller callback
template<class F>
router<F> route(const std::string& m, const std::string& p, F fun )
{
	return router<F>(m,p,fun);
}




//! register a GET HTTP route
//! \ingroup controller
template<class F>
auto GET(const std::string& p, F f)
{
	return route("GET",p,f);
}

//! register a POST HTTP route
//! \ingroup controller
template<class F>
auto POST(const std::string& p, F f)
{
	return route("POST",p,f);
}

//! register a DELETE HTTP route
//! \ingroup controller
template<class F>
auto DEL(const std::string& p, F f)
{
	return route("DELETE",p,f);
}

//! register a PUT HTTP route
//! \ingroup controller
template<class F>
auto PUT(const std::string& p, F f)
{
	return route("PUT",p,f);
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class F>
struct filter_router
{
	typedef F type;


	filter_router(const std::string& m, const std::string& p, F f, int prio = 0 )
		: method(m), path(p), handler(f),priority(prio)
	{}

	std::string method;
	std::string path;
	int priority;
	F handler;

	void ctx_register(diy::Context* ctx)
	{
		auto fc = ctx->resolve<FrontController>();
		registerFilter(*fc,method,path,handler,priority);
	}

private:

	template<class C>
	void registerFilter(FrontController& fc, const std::string& m, const std::string& p, void (C::*fun)( prio::Request&,  prio::Response&, std::shared_ptr<FilterChain> chain), int prio)
	{
		fc.registerFilter(m,p, [fun]( prio::Request& req,  prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(req,res,chain);
		},
		prio);
	}

	template<class C>
	void registerFilter(FrontController& fc, const std::string& m, const std::string& p, repro::Future<> (C::*fun)(prio::Request&, prio::Response&, std::shared_ptr<FilterChain> chain), int prio)
	{
		fc.registerFilter(m, p, [fun](prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(req, res, chain);
		},
		prio);
	}
};

//! register a HTTP filter
//! \ingroup controller
//!
//! \param m - HTTP method to filter (ie GET,POST)
//! \param p - HTTP path to be filtered. this is a regex
//! \param f - HTTP filter callback to call
//! \param priority - priority of filter relative to other filters
//! the callback will be called once request is complete but before the controller callback
template<class F>
filter_router<F> filter(const std::string& m, const std::string& p, F f, int priority = 0 )
{
	return filter_router<F>(m,p,f,priority);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class F>
struct completion_filter_router
{
	typedef F type;

	completion_filter_router(const std::string& m, const std::string& p, F f, int prio = 0 )
		: method(m), path(p), handler(f),priority(prio)
	{}

	std::string method;
	std::string path;
	int priority;
	F handler;

	void ctx_register(diy::Context* ctx)
	{
		auto fc = ctx->resolve<FrontController>();
		registerFilter(*fc,method,path,handler,priority);
	}

private:

	template<class C>
	void registerFilter(FrontController& fc, const std::string& m, const std::string& p, void (C::*fun)( prio::Request&,  prio::Response&, std::shared_ptr<FilterChain> chain), int prio)
	{
		fc.registerCompletionFilter(m,p, [fun]( prio::Request& req,  prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(req,res,chain);
		},
		prio);
	}

	template<class C>
	void registerFilter(FrontController& fc, const std::string& m, const std::string& p, repro::Future<> (C::*fun)(prio::Request&, prio::Response&, std::shared_ptr<FilterChain> chain), int prio)
	{
		fc.registerCompletionFilter(m, p, [fun](prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(req, res, chain);
		},
		prio);
	}
};

//! register a HTTP completion filter
//! \ingroup controller
//!
//! \param m - HTTP method to filter (ie GET,POST)
//! \param p - HTTP path to be filtered. this is a regex
//! \param f - HTTP filter callback to call
//! \param priority - priority of filter relative to other filters
//! the callback will be called once response is completely send
//! you cannot modify the response any more
template<class F>
completion_filter_router<F> completion_filter(const std::string& m, const std::string& p, F f, int priority = 0 )
{
	return completion_filter_router<F>(m,p,f,priority);
}

template<class F>
struct flush_filter_router
{
	typedef F type;

	flush_filter_router(const std::string& m, const std::string& p, F f, int prio = 0 )
		: method(m), path(p), handler(f),priority(prio)
	{}

	std::string method;
	std::string path;
	int priority;
	F handler;

	void ctx_register(diy::Context* ctx)
	{
		auto fc = ctx->resolve<FrontController>();
		registerFilter(*fc,method,path,handler,priority);
	}

private:

	template<class C>
	void registerFilter(FrontController& fc, const std::string& m, const std::string& p, void (C::*fun)( prio::Request&,  prio::Response&, std::shared_ptr<FilterChain> chain), int prio)
	{
		fc.registerFlushFilter(m,p, [fun]( prio::Request& req,  prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(req,res,chain);
		},
		prio);
	}

	template<class C>
	void registerCompletionFilter(FrontController& fc, const std::string& m, const std::string& p, repro::Future<> (C::*fun)(prio::Request&, prio::Response&, std::shared_ptr<FilterChain> chain), int prio)
	{
		fc.registerFlushFilter(m, p, [fun](prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(req, res, chain);
		},
		prio);
	}
};

//! register a HTTP flush filter
//! \ingroup controller
//!
//! \param m - HTTP method to filter (ie GET,POST)
//! \param p - HTTP path to be filtered. this is a regex
//! \param f - HTTP filter callback to call
//! \param priority - priority of filter relative to other filters
//! the callback will be called once response has been build when flush() is called
//! the response can still be modified

template<class F>
flush_filter_router<F> flush_filter(const std::string& m, const std::string& p, F f, int priority = 0 )
{
	return flush_filter_router<F>(m,p,f,priority);
}

//////////////////////////////////////////////////////////////

//! returns a lambda [](const std::exception& ex){...} that will return a HTTP error code when called
//! \ingroup controller
inline auto render_error(prio::Response& res)
{
	return [&res] (const std::exception& ex) 
	{
		res.error().body(ex.what()).flush();	
	};
}

//! \private
// terminator for below recursive function
inline void redirect_url(std::ostringstream& oss)
{
}

//! \private redirect url recursively
template<class T,class ... Args>
void redirect_url(std::ostringstream& oss, T&& t, Args&& ... args)
{
	oss << t;
	redirect_url(oss,std::forward<Args>(args)...);
}

//! redirect url constructed  from args containing path fragments
//! \ingroup controller
template<class ... Args>
inline auto redirect(prio::Response& res, Args ... args)
{
	std::ostringstream oss;
	redirect_url(oss,std::forward<Args>(args)...);

	std::string url = oss.str();

	return [&res,url](auto ... params)
	{
		std::ostringstream oss;
		oss << url;
		redirect_url(oss,params...);
		res.redirect(oss.str()).flush();
	};
}


//////////////////////////////////////////////////////////////

template<class F>
class exception_handler
{
public:

	typedef F type;


	exception_handler(F f)
		: fun_(f)
	{}

	void ctx_register(diy::Context* ctx)
	{
		auto fc = ctx->resolve<FrontController>();
		registerExHandler(*fc,fun_);
	}

private:

	F fun_;

	template<class C,class E>
	void registerExHandler(FrontController& fc, void (C::*fun)( const E& ex, prio::Request&,  prio::Response&))
	{
		fc.registerExceptionHandler<E>( [fun]( const E& ex, prio::Request& req,  prio::Response& res)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(ex,req,res);
		});
	}


	template<class C, class E>
	void registerExHandler(FrontController& fc, repro::Future<> (C::*fun)(const E& ex, prio::Request&, prio::Response&))
	{
		fc.registerExceptionHandler<E>([fun](const E& ex, prio::Request& req, prio::Response& res)
		{
			auto ptr = req.attributes.attr<std::shared_ptr<diy::Context>>("ctx")->resolve<C>();
			(*ptr.*fun)(ex, req, res);
		});
	}
};

//! specify global exception handler class
//! \ingroup controller
template<class F>
exception_handler<F> ex_handler(F fun)
{
	return exception_handler<F>(fun);
}

//////////////////////////////////////////////////////////////
//! WebApplicationContext
//////////////////////////////////////////////////////////////

//! \ingroup webserver

class WebApplicationContext : public diy::Context
{
public:

	template<class ... Args>
	WebApplicationContext(Args&& ... args)
		: diy::Context(nullptr)
	{
		// make Context itself injectable
		std::shared_ptr<diy::Context> ctx = std::shared_ptr<diy::Context>( 
			(diy::Context*)this, 
			[](diy::Context* c) {}
		);

		register_static<WebApplicationContext,Context>(ctx);

		// register a FrontController
		auto fc = std::make_shared<FrontController>(ctx);
		register_static<FrontController()>(fc);

		register_dependencies<Args&&...>(std::forward<Args>(args)...);
	}

private:

	template<class ... Args>
	void register_dependencies()
	{}

	template<class T, class ... Args>
	void register_dependencies(T&& t,Args&& ... args)
	{
		t.ctx_register(this);
		register_dependencies<Args&&...>(std::forward<Args>(args)...);
	}

	WebApplicationContext(const WebApplicationContext& rhs) = delete;
};

typedef diy::Injector http_routes;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

}

#endif

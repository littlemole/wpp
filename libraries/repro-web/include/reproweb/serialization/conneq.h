#ifndef _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_JSON_XML_CONEQ_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_JSON_XML_CONEQ_DEF_GUARD_

#include <reproweb/serialization/xml.h>
#include <reproweb/serialization/json.h>

namespace reproweb {

//////////////////////////////////////////////////////////////

template<class T>
struct entity
{
	T value;

	entity() {}
	entity(T t)
		: value(std::move(t))
	{}

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
using async_t = repro::Future<entity<T>>;

template<class T>
auto async_entity()
{
	return repro::promise<entity<T>>();
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

template<class T>
class HandlerParam<entity<T>,typename std::enable_if<std::is_class<T>::value>::type>
{
public:

	static entity<T> get(prio::Request& req,  prio::Response& res)
	{
        entity<T> t;

        if(req.headers.content_type() == "application/xml")
        {
            meta::fromXml(req.body(),t.value);
        }
        else
        {
            meta::fromJson(req.body(),t.value);
        }

        validate(t);
		return t;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////

inline bool wantXml(prio::Request& req,  prio::Response& res)
{
    if ( res.headers.content_type() == "application/xml")
        return true;

    if( req.headers.accept() == "application/xml")
        return true;

    return false;
}

template<class T>
void output_conneq(prio::Request& req,  prio::Response& res,T& t)
{
    if(wantXml(req,res))
    {
        res.contentType("application/xml");
        output_xml(res,t);
    }
    else
    {
        res.contentType("application/json");
        output_json(res,t);
    }
}

//////////////////////////////////////////////////////////////


template<class R,class C, class ... Args>
Async invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, repro::Future<entity<R>> (C::*fun)(Args...) )
{
	try
	{
		C& c = prepare_controller<C>(req);
		entity<R> r = co_await (c.*fun)(HandlerParam<Args>::get(req,res)...);		
		output_conneq(req,res,r.value);
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


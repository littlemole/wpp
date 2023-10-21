#ifndef _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_XML_XXX_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_XML_XXX_DEF_GUARD_


#include <patex/document.h>
#include <metacpp/xml.h>
#include <reproweb/serialization/parameter.h>
#include "reproweb/ctrl/front_controller.h"


namespace reproweb {

//////////////////////////////////////////////////////////////

template<class T>
struct xml_t
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
using async_xml_t = repro::Future<xml_t<T>>;


template<class T>
auto xml_promise()
{
	return repro::promise<xml_t<T>>();
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class T>
class HandlerParam<xml_t<T>>
{
public:

	static xml_t<T> get(prio::Request& req,  prio::Response& /*res*/ )
	{
		auto doc = patex::xml::Document::parse_str(req.body());

		xml_t<T> t;
		meta::fromXml(doc,t.value);
		validate(t.value);

		return t;
	}
};

//////////////////////////////////////////////////////////////


inline void output_xml(prio::Response& res,const std::string& xml)
{
	res
	.body(xml)
	.contentType("application/xml")
	.ok()
	.flush();
}

template<class T>
void output_xml(prio::Response& res, T& t)
{
	auto doc = meta::toXml(t);
	output_xml(res, doc->toString() );
}


template<class T>
void output_xml(prio::Response& res, xml_t<T>& t)
{
	auto doc = meta::toXml(t.value);
	output_xml(res, doc->toString() );
}


//////////////////////////////////////////////////////////////


template<class R,class C, class ... Args>
Async invoke_handler(FrontController& fc, prio::Request& req,  prio::Response& res, repro::Future<xml_t<R>> (C::*fun)(Args...) )
{
	try
	{
		C& c = prepare_controller<C>(req);
		xml_t<R> r = (c.*fun)(HandlerParam<Args>::get(req,res)...);	
		output_xml(res,r);
	}
	catch(std::exception& ex)
	{
		fc.handle_exception(ex, req, res);
	}

	(void)(co_await prio::nextTick());
	co_return;
}


}

#endif


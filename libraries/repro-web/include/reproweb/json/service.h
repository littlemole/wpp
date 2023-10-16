#ifndef MOL_DEF_GUARD_DEFINE_REPROWEB_JSON_SERVICE_H_DEF_GUARD
#define MOL_DEF_GUARD_DEFINE_REPROWEB_JSON_SERVICE_H_DEF_GUARD

#include "reproweb/json/json.h"
#include "reprocurl/api.h"

namespace reproweb {


class RestEx : public repro::ReproEx<RestEx>	
{
public:

	RestEx() : status(500), json(Json::nullValue) {};
	RestEx(long s) : status(s),json(Json::nullValue)  {};
	RestEx(long s,const std::string& b) : repro::ReproEx<RestEx>(b),status(s),json(Json::nullValue)  {};
	RestEx(long s,Json::Value j) : status(s),json(j)  
	{
		if(json.isMember("error"))
		{
			auto e = json["error"];
			if (e.isMember("msg"))
			{
				msg = e["msg"].asString();
			}
		}
	};

	long status;
	Json::Value json;
};

struct Rest
{
	static Rest url(const std::string& serviceUrl)
	{
		Rest r;
		r.req_.url(serviceUrl);

		return r;
	}

    template<class T, class ... Args>
	static Rest url(const std::string& serviceUrl, T t, Args ... args)
	{
        std::ostringstream oss;
        oss << serviceUrl;
        if( serviceUrl.empty() || serviceUrl.back() != '/')
        {
            oss << "/";
        }
        oss << t;
		return url(oss.str(),args ...);
	}

	Rest& cert(const std::string& crt)
	{
		req_.client_cert(crt);
		return *this;
	}

	Rest& insecure()
	{
		req_.insecure();
		return *this;
	}

	Rest& verbose()
	{
		req_.verbose();
		return *this;
	}

	Rest& method(const std::string& m)
	{
		req_.method(m);
		return *this;
	}	

	Rest& get()
	{
		req_.method("GET");
		return *this;
	}
	
	Rest& remove()
	{
		req_.method("DELETE");

		return *this;	
	}	

	template<class T>
	Rest& post(T& t)
	{
		req_.data( ::JSON::flatten(meta::toJson(t))).method("POST");

		return *this;	
	}

	template<class T>
	Rest& put(T& t)
	{
		req_.data( ::JSON::flatten(meta::toJson(t))).method("PUT");

		return *this;	
	}


	template<class O>
	repro::Future<O> fetch()
	{
		return fetch<O>(req_);
	}

	repro::Future<> call()
	{
		return call(req_);
	}


private:

	reprocurl::request req_;

	template<class O>
	static repro::Future<O> fetch(reprocurl::request& req)
	{
		auto p = repro::promise<O>();
				
		reprocurl::fetch(req)
		.then([p](reprocurl::response res)
		{
			std::cout << res.status() << ":" << res.content() << std::endl;

			Json::Value json = parse(res);
			O o;
			meta::fromJson(json,o);

			p.resolve(o);
		})
		.otherwise(repro::reject(p));

		return p.future();			
	}

	static repro::Future<> call(reprocurl::request& req)
	{
		auto p = repro::promise<>();
				
		reprocurl::fetch(req)
		.then([p](reprocurl::response res)
		{
			std::cout << res.status() << ":" << res.content() << std::endl;

			Json::Value json = parse(res);

			p.resolve();
		})
		.otherwise(repro::reject(p));

		return p.future();			
	}	

	static Json::Value parse(reprocurl::response& res)
	{
		Json::Value json;
		try {
			json = ::JSON::parse(res.content());
		}
		catch(...)
		{
			json = Json::nullValue;
		}

		if(res.status() != 200)
		{
			if(!json.isNull())
			{
				throw RestEx(res.status(),json);	 
			}
			else
			{
				throw RestEx(res.status(),res.content());	
			}
		}		
		return json;
	}

};

}


#endif

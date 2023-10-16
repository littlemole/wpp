#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VALID_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VALID_CONTROLLER_DEFINE_

#include "reproweb/tools/validation.h"
#include "entities.h"

using namespace reproweb;
using namespace repro;
using namespace prio;


class Valid
{
public:

	static const std::string username( QueryParams& params)
	{
		static std::regex r("[^<>]*");

		std::string username = params.get("username");

		if(username.empty())
			throw RegistrationEx("error.msg.username.empty");

		return valid<RegistrationEx>(username, r, "error.msg.username.invalid");
	}

	template<class E>
	static const std::string passwd( QueryParams& params)
	{
		static std::regex r(".*");

		std::string pwd = params.get("pwd");

		return valid<E>(pwd, r , "error.msg.password.empty");
	}

	template<class E>
	static const std::string login( QueryParams& params)
	{
		std::string login = params.get("login");

		if(login.empty())
			throw E("error.msg.login.empty");

		return valid_email<E>(login, "error.msg.login.invalid.email" );
	}

	static const std::string avatar( QueryParams& params)
	{
		std::string avatar = params.get("avatar_url");

		if(avatar.empty())
			return "https://upload.wikimedia.org/wikipedia/commons/e/e4/Elliot_Grieveson.png";

		return valid_url<RegistrationEx>(avatar, "error.msg.avatar.invalid.url" );
	}	

	static std::string locale(Request& req)
	{
		auto h = req.headers.values("Accept-Language");
		auto lang = h.value().main();
		std::string locale = std::regex_replace (lang,std::regex("-"),"_");		

		return locale;
	}    
};


#endif
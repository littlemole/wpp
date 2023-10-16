#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_

#include "reproweb/tools/config.h"
#include "reproweb/json/json.h"
#include <reproweb/serialization/json.h>
#include <reproweb/ctrl/session.h>
#include "valid.h"

class User
{
public:

	User() {}

	User( 
		const std::string& name,
		const std::string& login,
		const std::string& avatar_url
	)
	  :  name_(name),
		 login_(login),
		 avatar_url_(avatar_url)
	{}

	std::string username() const 	{ return name_; }
	std::string login() const 	  	{ return login_; }
	std::string avatar_url() const  { return avatar_url_; }

	static constexpr auto meta()
	{
		return meta::data(
			"login", &User::login_,
			"username", &User::name_,
			"avatar_url", &User::avatar_url_
		);
	}
	
private:
	std::string name_;	
	std::string login_;	
	std::string avatar_url_;	
};

/*
class Session
{
public:
	Session()
	{}

	Session(Json::Value profile) 
		:sid_(make_session_id()), profile_(profile)
	{}

	Session(const std::string& sid,Json::Value profile) 
		:sid_(sid), profile_(profile)
	{}

	std::string sid() const  	{ return sid_; }
	Json::Value profile() const { return profile_; }
		

private:
	std::string sid_;
	Json::Value profile_;	

	static std::string make_session_id()
	{
		std::string sid = "repro_web_sid::";
		sid += cryptoneat::toHex(cryptoneat::nonce(64));
		return sid;
	}
};

auto meta(const ::Session&)
{
	return metadata(
		getter_setter("sid",&::Session::sid),
		getter_setter("profile",&::Session::profile)
	);
}
*/

class AppConfig : public reproweb::Config
{
public:
	AppConfig()
	  : reproweb::Config("config.json")
	{
		const char* redis = getenv("REDIS_HOST");
		if(redis)
		{
			std::ostringstream oss;
			oss << "redis://" << redis << ":6379";

			get("redis") = oss.str();
		}
		std::cout << "REDIS: " << get("redis") << std::endl;
	}
};

#endif

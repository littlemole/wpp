#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_

#include "reproweb/tools/config.h"
#include "reproweb/tools/validation.h"
#include "reproweb/serialization/json.h"
#include "reproweb/serialization/web.h"

using namespace reproweb;
using namespace repro;
using namespace prio;


class NoSessionEx : public repro::ReproEx<NoSessionEx> 
{
public:
	NoSessionEx() {}
	NoSessionEx(const std::string& s) : repro::ReproEx<NoSessionEx> (s) {}
};


class SessionId
{
public:

	std::string sid;

	static constexpr auto meta()
	{
		return meta::data(
			"sid", 		&SessionId::sid
		);
	}	

	void validate()
	{
		static std::regex r("repro_web_sid::[0-9a-f]*");

		if(sid.empty())
			throw NoSessionEx("no session found");

		valid<NoSessionEx>( 
			sid, 
			r,
			"invalid session id."
		);			
	}
};

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
			meta::entity_root("user"),
			"login", 		&User::login_,
			"username", 	&User::name_,
			"avatar_url", 	&User::avatar_url_
		);
	}
	
private:
	std::string name_;	
	std::string login_;	
	std::string avatar_url_;	
};
 


class Session
{
public:
	Session()
	{}

	Session(User profile) 
		:sid_(make_session_id()), profile_(profile)
	{}

	Session(const std::string& sid, User profile) 
		:sid_(sid), profile_(profile)
	{}

	std::string sid() const  	{ return sid_; }
	User profile() const 		{ return profile_; }

	static constexpr auto meta()
	{
		return meta::data(
			"sid", 		&Session::sid_,
			"profile", 	&Session::profile_
		);
	}		

private:
	std::string sid_;
	User profile_;	

	static std::string make_session_id()
	{
		std::string sid = "repro_web_sid::";
		sid += cryptoneat::toHex(cryptoneat::nonce(64));
		return sid;
	}
};


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

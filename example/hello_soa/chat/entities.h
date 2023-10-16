#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_


#include "reproweb/tools/config.h"
#include "reproweb/json/json.h"
#include <reproweb/ctrl/controller.h>
#include <reproweb/serialization/json.h>

using namespace prio;
using namespace repro;
using namespace reproweb;

MAKE_REPRO_EX(AuthEx)
MAKE_REPRO_EX(LoginEx)
MAKE_REPRO_EX(RegistrationEx)


class User
{
public:

	User() {}

	User( 
		const std::string& name,
		const std::string& login,
		const std::string& hash,
		const std::string& avatar_url
	)
	  :  name_(name),
		 login_(login),
		 hash_(hash),
		 avatar_url_(avatar_url)
	{}

	std::string username() const 	  { return name_; }
	std::string login() const 	  { return login_; }
	std::string hash() const  	  { return hash_; }
	std::string avatar_url() const  { return avatar_url_; }

	static constexpr auto meta()
	{
		return meta::data(
			"login", &User::login_,
			"username", &User::name_,
			"pwd", &User::hash_,
			"avatar_url", &User::avatar_url_
		);
	}
	
private:
	std::string name_;	
	std::string login_;	
	std::string hash_;	
	std::string avatar_url_;	
};


#define TO_STR_HELPER(x) #x
#define TO_STR(x) TO_STR_HELPER(x)

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
				
		const char* user = getenv("USER_SERVICE_HOST");
		if(user)
		{
			std::ostringstream oss;
			oss << "https://" << user << ":9877";

			get("user-service") = oss.str();
		}
		std::cout << "USER_SERVICE: " << get("user-service") << std::endl;

		const char* session = getenv("SESSION_SERVICE_HOST");
		if(session)
		{
			std::ostringstream oss;
			oss << "https://" << session << ":9878";

			get("session-service") = oss.str();
		}
		std::cout << "SESSION_SERVICE: " << get("session-service") << std::endl;

		json()["version"] = TO_STR(VERSION);

		std::ostringstream oss1;
		oss1 << getString("session-service") << "/session";
		sessionService_ = oss1.str();		

		std::ostringstream oss2;
		oss2 << getString("user-service") << "/register";
		registrationService_ = oss2.str();		

		std::ostringstream oss3;
		oss3 << getString("user-service") << "/login";
		loginService_ = oss3.str();		
	}
/*
	std::string sessionService(const std::string& sid)
	{
		std::ostringstream oss;
		oss << getString("session-service") << "/session/" << sid;
		return oss.str();
	}
*/
	std::string sessionEndpoint()
	{
		return sessionService_;
	}		

	std::string registrationEndpoint()
	{
		return registrationService_;
	}	

	std::string loginEndpoint()
	{
		return loginService_;
	}		

	std::string cert()
	{
		return getString("cert");
	}

private:

	std::string sessionService_;
	std::string registrationService_;
	std::string loginService_;
};

#endif

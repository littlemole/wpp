#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_

#include <string>
#include <memory>

#include "reproweb/tools/config.h"

class AuthEx : public repro::ReproEx<AuthEx> 
{
public:
	AuthEx() {}
	AuthEx(const std::string& s) : ReproEx<AuthEx>(s) {}
};

class LoginEx : public repro::ReproEx<LoginEx> 
{
public:
	LoginEx() {}
	LoginEx(const std::string& s) : ReproEx<LoginEx> (s) {}
};

class RegistrationEx : public repro::ReproEx<RegistrationEx> 
{
public:
	RegistrationEx() {}
	RegistrationEx(const std::string& s) : ReproEx<RegistrationEx>(s) {}
};



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

	Json::Value toJson() const
	{
		Json::Value result(Json::objectValue);
		result["username"] = name_;
		result["login"] = login_;
		result["avatar_url"] = avatar_url_;
		return result;
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

		json()["version"] = TO_STR(VERSION);
	}
};

#endif

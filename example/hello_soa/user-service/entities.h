#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_

#include "reproweb/json/json.h"
#include "reproweb/tools/config.h"
#include "reproweb/tools/validation.h"
#include <reproweb/ctrl/controller.h>
#include <reproweb/serialization/json.h>
#include <reproweb/serialization/xml.h>
#include <reproweb/serialization/web.h>
#include <metacpp/meta.h>

using namespace reproweb;
using namespace repro;
using namespace prio;


MAKE_REPRO_EX(BadRequestEx)
MAKE_REPRO_EX(UserNotFoundEx)
MAKE_REPRO_EX(LoginEx)
MAKE_REPRO_EX(LoginAlreadyTakenEx)
MAKE_REPRO_EX(RegistrationEx)


class Valid 
{
public:

	static const std::string username( const std::string& name)
	{
		static std::regex r("[^<>]*");

		if(name.empty())
			throw BadRequestEx("error.msg.username.empty");

		return valid<BadRequestEx>(name, r, "error.msg.username.invalid");
	}

	static const std::string passwd( const std::string& pwd)
	{
		static std::regex r(".*");

		return valid<BadRequestEx>(pwd, r , "error.msg.password.empty");
	}

	static const std::string login( const std::string& email)
	{
		static std::regex r("^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\\.[a-zA-Z0-9-]+)*$");

		if(email.empty())
			throw BadRequestEx("error.msg.login.empty");

		return valid<BadRequestEx>(email, r, "error.msg.login.invalid.email" );
	}

	static const std::string avatar( const std::string& url)
	{
		static std::regex r("(http|https)://(\\w+:{0,1}\\w*@)?(\\S+)(:[0-9]+)?(/|/([\\w#!:.?+=&%@!-/]))?");

		if(url.empty())
			return "https://upload.wikimedia.org/wikipedia/commons/e/e4/Elliot_Grieveson.png";

		return valid<BadRequestEx>(url, r, "error.msg.avatar.invalid.url" );
	}	
   
};

class Login
{
public:

	Login() {}

	Login( 
		const std::string& login,
		const std::string& hash
	)
	  :  login_(login),
		 hash_(hash)
	{}

	std::string login() const 	  		{ return login_; }
	std::string hash() const  	 		{ return hash_; }

	void validate()
	{
		Valid::login(login_);
		Valid::passwd(hash_);
	}

	static constexpr auto meta()
	{
		return meta::data (
			"login",&Login::login_,
			"pwd", &Login::hash_
		);
	}

protected:
	std::string login_;	
	std::string hash_;	
};


class User : public Login
{
public:

	User() {}

	User( 
		const std::string& name,
		const std::string& login,
		const std::string& hash,
		const std::string& avatar_url
	)
	  :  Login(login,hash),
	  	 name_(name),
		 avatar_url_(avatar_url)
	{}

	std::string username() const 	  		{ return name_; }
	std::string avatar_url() const  	 	{ return avatar_url_; }

	void validate()
	{
		Valid::login(login_);
		Valid::passwd(hash_);
		Valid::username(name_);
		Valid::avatar(avatar_url_);
	}

	static constexpr auto meta()
	{
		return ::meta::data (
			"username", &User::name_,
			"login",&User::login_,
			"pwd", &User::hash_,
			"avatar_url", &User::avatar_url_
		);
	}

private:
	std::string name_;	
	std::string avatar_url_;	
};


class AppConfig : public reproweb::Config
{
public:
	AppConfig()
	  : Config("config.json")
	{}
};

#endif

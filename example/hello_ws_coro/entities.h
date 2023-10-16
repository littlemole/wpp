#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_

#include <string>
#include <memory>

#include "reproweb/tools/config.h"
#include "cryptoneat/cryptoneat.h"
#include "reproweb/tools/validation.h"
#include <reproweb/serialization/web.h>
#include <reproweb/serialization/json.h>

using namespace prio;
using namespace repro;
using namespace reproweb;

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

	std::string login() const 	  	{ return login_; }
	std::string hash() const  	  	{ return hash_; }

	static constexpr auto meta()
	{
		return meta::data(
			"login",		&Login::login_,
			"pwd",			&Login::hash_
		);
	}	

	void validate()
	{
		static std::regex r_email("^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\\.[a-zA-Z0-9-]+)*$");
		static std::regex r_pwd(".*");

		if(login_.empty())
			throw LoginEx("error.msg.login.empty");

		valid<LoginEx>(login_, 		r_email, "error.msg.login.invalid.email" );
		valid<LoginEx>(hash_, 		r_pwd , "error.msg.password.empty");
	}
	
private:
	std::string login_;	
	std::string hash_;	
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

	std::string username() const 	{ return name_; }
	std::string login() const 	  	{ return login_; }
	std::string hash() const  	  	{ return hash_; }
	std::string avatar_url() const  { return avatar_url_; }
	
	static constexpr auto meta()
	{
		return meta::data(
			"username", 	&User::name_,
			"login",		&User::login_,
			"pwd",			&User::hash_,
			"avatar_url",	&User::avatar_url_
		);
	}	

	void validate()
	{
		static std::regex r_username("[^<>]*");
		static std::regex r_email("^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\\.[a-zA-Z0-9-]+)*$");
		static std::regex r_pwd(".*");
		static std::regex r_url("(http|https)://(\\w+:{0,1}\\w*@)?(\\S+)(:[0-9]+)?(/|/([\\w#!:.?+=&%@!-/]))?");

		if(name_.empty())
			throw RegistrationEx("error.msg.username.empty");

		if(login_.empty())
			throw RegistrationEx("error.msg.login.empty");

		if(avatar_url_.empty())
			avatar_url_= "https://upload.wikimedia.org/wikipedia/commons/e/e4/Elliot_Grieveson.png";


		valid<RegistrationEx>(name_, 		r_username, "error.msg.username.invalid");
		valid<RegistrationEx>(login_, 		r_email, "error.msg.login.invalid.email" );
		valid<RegistrationEx>(hash_, 		r_pwd , "error.msg.password.empty");
		valid<RegistrationEx>(avatar_url_, 	r_url, "error.msg.avatar.invalid.url" );		
	}

private:
	std::string name_;	
	std::string login_;	
	std::string hash_;	
	std::string avatar_url_;	
};


#endif

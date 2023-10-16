#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_ENTITIES_DEFINE_

#include "reproweb/tools/config.h"
#include "reproweb/json/json.h"
#include "reproweb/serialization/json.h"
#include "reproweb/serialization/web.h"
#include "reproweb/tools/validation.h"

using namespace reproweb;
using namespace repro;
using namespace prio;

class HttpEx : public repro::Ex				
{											
public:										
	HttpEx(const char* s) 
		: status_(s) 
	{}									
	
	HttpEx(const char* s,const std::string& m) 				
		: repro::Ex(m),status_(s) 
	{}	

	virtual void render_error(Response& res) const		
	{										
		Json::Value json = meta::exToJson(*this);	
		res									
		.body(JSON::flatten(json))			
		.status(status_)						
		.contentType("application/json")	
		.flush();							
	}	
	virtual std::exception_ptr make_exception_ptr() const
	{
		return std::make_exception_ptr(*this);
	}

protected:
	const char* status_;										
};

template<class E>
class HttpTplEx : public HttpEx				
{											
public:										
	HttpTplEx(const char* s) 
		: HttpEx(s) 
	{}									
	
	HttpTplEx(const char* s,const std::string& m) 				
		: HttpEx(s,m) 
	{}	

	virtual void render_error(Response& res) const		
	{			
		E* e = (E*) this;					

		Json::Value json = meta::exToJson(*e);	
		res									
		.body(JSON::flatten(json))			
		.status(status_)						
		.contentType("application/json")	
		.flush();							
	}	

	virtual std::exception_ptr make_exception_ptr() const
	{
		E* e = (E*) this;

		return std::make_exception_ptr(*e);
	}
};

#define MAKE_REPRO_HTTP_EX(ex,code) 		\
class ex : public HttpTplEx<ex>				\
{											\
public:										\
	ex() : HttpTplEx<ex>(code) {}			\
	ex(const std::string& s) 				\
	: HttpTplEx<ex>(code,s) {}				\
};

MAKE_REPRO_HTTP_EX(BadRequestEx,"HTTP/1.1 400")
MAKE_REPRO_HTTP_EX(UserNotFoundEx,"HTTP/1.1 404")
MAKE_REPRO_HTTP_EX(LoginEx,"HTTP/1.1 401")
MAKE_REPRO_HTTP_EX(LoginAlreadyTakenEx,"HTTP/1.1 400")
MAKE_REPRO_HTTP_EX(RegistrationEx,"HTTP/1.1 400")

MAKE_REPRO_HTTP_EX(ServerEx,"HTTP/1.1 500")


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

class Email
{
public:

	std::string value;

	static constexpr auto meta()
	{
		return meta::data(
			"email", 		&Email::value
		);
	}	

	void validate()
	{
		Valid::login(value);
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
		return meta::data(
			meta::entity_root("login"),
			"login", 		&Login::login_,
			"pwd", 			&Login::hash_
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
		Login::validate();
		Valid::username(name_);
		Valid::avatar(avatar_url_);
	}

	static constexpr auto meta()
	{
		return meta::data(
			meta::entity_root("user"),
			"username", 	&User::name_,
			"login", 		&User::login_,
			"pwd", 			&User::hash_,
			"avatar_url", 	&User::avatar_url_
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

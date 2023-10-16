#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "reproweb/ctrl/session.h"
#include "reproweb/json/service.h"
#include "entities.h"

using namespace prio;
using namespace repro;
using namespace reproweb;


class SessionService
{
public:

	SessionService(std::shared_ptr<AppConfig> conf)
		: config(conf)
	{}

	Future<reproweb::Session> get_user_session( std::string sid)
	{
		auto p = promise<reproweb::Session>();

		reproweb::Rest::url( config->sessionEndpoint(), sid )  
		//.insecure()
		.get() 
		.fetch<Json::Value>() 
		.then([p,sid](Json::Value json)
		{
			Session session;
			meta::fromJson(json,session);
			p.resolve(session);
		})
		.otherwise([p,sid](const std::exception& ex)
		{
			std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;
			p.reject(AuthEx(ex.what()));
		});

		return p.future();
	}

	Future<> write_user_session(reproweb::Session& session)
	{
		auto p = promise<>();

		
		reproweb::Rest::url( config->sessionEndpoint() )
		//.insecure()
		.post( session )
		.fetch<Json::Value>()
		.then([p](Json::Value json)
		{
			p.resolve();
		})
		.otherwise( [p](const std::exception& ex)
		{
			std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;
			p.reject(AuthEx(ex.what()));
		}); 

		return p.future();
	}

	Future<> remove_user_session( std::string sid)
	{
		auto p = promise<>();

		reproweb::Rest::url( config->sessionEndpoint(), sid )
		//.insecure()
		.remove( )
		.call()
		.then([p]()
		{
			p.resolve();
		})
		.otherwise( [p](const std::exception& ex)
		{
			std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;
			p.resolve();
		}); 

		return p.future();
	}

private:

	std::shared_ptr<AppConfig> config;
};


class UserService
{
public:

	UserService(std::shared_ptr<AppConfig> conf)
		: config(conf)
	{}

	Future<User> register_user( User user )
	{
		auto p = promise<User>();

		reproweb::Rest::url( config->registrationEndpoint())
		//.insecure()
		.cert(config->cert())
		.post( user )
		.fetch<Json::Value>()
		.then([p](Json::Value json)
		{
			User user;
			meta::fromJson(json,user);

			p.resolve(user);
		})
		.otherwise( [p](const std::exception& ex)
		{
			std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;
			p.reject(RegistrationEx(ex.what()));
		}); 

		return p.future();
	}

	Future<User> login_user( const std::string& login, const std::string& pwd )
	{
		auto p = promise<User>();

		Json::Value json(Json::objectValue);
		json["login"] = login;
		json["pwd"] = pwd;

		reproweb::Rest::url( config->loginEndpoint())
		//.insecure()
		.cert(config->cert())
		.post(json)
		.fetch<Json::Value>()
		.then([p](Json::Value json)		
		{
			User user;
			meta::fromJson(json,user);

			p.resolve(user);
		})
		.otherwise( [p](const std::exception& ex)
		{
			std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;
			p.reject(LoginEx(ex.what()));
		}); 

		return p.future();
	}
private:

	std::shared_ptr<AppConfig> config;

};



#endif

#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "entities.h"
#include "reproweb/json/service.h"

using namespace prio;
using namespace repro;
using namespace reproweb;


class SessionService
{
public:

	SessionService(std::shared_ptr<AppConfig> conf)
		: config(conf)
	{}

	Future<::Session> get_user_session( std::string sid)
	{
		User user;
		
		try
		{
			user = co_await Rest::url( config->sessionEndpoint(), sid )
			.insecure()
			.get()
			.fetch<User>();
		}
		catch(const std::exception& e)
		{
			throw AuthEx();
		}
		
		co_return ::Session(sid,user);
	}

	Future<::Session> write_user_session(User user)
	{
		::Session session = co_await Rest::url( config->sessionEndpoint() )
		.insecure()
		.post(user)
		.fetch<::Session>();

		co_return session;	
	}

	Future<> remove_user_session( std::string sid)
	{
		co_await Rest::url( config->sessionEndpoint(), sid )
		.insecure()
		.remove()
		.call();
		
		co_return;
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
		User result;
		
		try 
		{
			result = co_await Rest::url(config->registrationEndpoint())
			.insecure()
			.post(user)
			.fetch<User>();
		}
		catch(RestEx& ex)
		{
			throw RegistrationEx(ex.json["error"]["msg"].asString());
		}
		co_return result;
	}

	Future<User> login_user( Login login )
	{
		User result;
		try 
		{
			result = co_await Rest::url( config->loginEndpoint())
			.insecure()
			.post( login )
			.fetch<User>();
		}
		catch(RestEx& ex)
		{
			throw LoginEx(ex.json["error"]["msg"].asString());
		}
		
		co_return result;
	}
private:

	std::shared_ptr<AppConfig> config;

};



#endif

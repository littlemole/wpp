#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_

#include "service.h"

class Model 
{
public:
	Model( std::shared_ptr<SessionService> session, std::shared_ptr<UserService> user)
		:  sessionService(session), 
		   userService(user)
	{}


	Future<Json::Value> chat( const std::string& sid )
	{
  		::Session session = co_await sessionService->get_user_session(sid);
		
		co_return meta::toJson(session.profile());
	}

	Future<std::string> login( Login login )
	{
		User user = co_await userService->login_user(login);

		::Session session = co_await sessionService->write_user_session(user);

		co_return session.sid();
	}

	Future<> logout( const std::string& sid )
	{
		return sessionService->remove_user_session(sid);
	}

	Future<std::string> register_user( User& user )
	{
		User result = co_await userService->register_user(user);

		::Session session = co_await sessionService->write_user_session(result);

		co_return session.sid();
	}

private:

	std::shared_ptr<SessionService>  sessionService;
	std::shared_ptr<UserService> 	 userService;
};

#endif
#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_

#include "entities.h"
#include "service.h"

class Model 
{
public:
	Model( std::shared_ptr<UserService> user)
		:  userService(user)
	{}


	Future<User> login( std::string login, std::string pwd )
	{
		return userService->login_user(login,pwd);
	}

	Future<User> register_user( User& user )
	{
		return userService->register_user(user);
	}

private:

	std::shared_ptr<UserService> 	 userService;
};

#endif
#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_

#include <string>
#include <memory>

#include "entities.h"
#include "repo.h"


#define TO_STR_HELPER(x) #x
#define TO_STR(x) TO_STR_HELPER(x)

class AppConfig : public Config
{
public:
	AppConfig()
	  : Config("config.json")
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

class Model 
{
public:
	Model( std::shared_ptr<UserRepository> userRepo)
		:  userRepository(userRepo)
	{}


	Future<User> login( Login creds )
	{
		User user = co_await userRepository->get_user( creds.login() );

		cryptoneat::Password pass;
		bool verified = pass.verify( creds.hash(), user.hash() );
		if(!verified) 
		{
			throw LoginEx("error.msg.login.failed");
		}

		co_return user;
	}

	Future<User> register_user( User user )
	{
		User result = co_await userRepository->register_user(
			user.username(), 
			user.login(), 
			user.hash(),
			user.avatar_url()
		);

		co_return result;
	}

private:

	std::shared_ptr<UserRepository> userRepository;
};

#endif

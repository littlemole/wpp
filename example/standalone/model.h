#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_MODEL_DEFINE_

#include "entities.h"
#include "repo.h"

#include "cryptoneat/cryptoneat.h"

class Users 
{
public:
	Users( std::shared_ptr<UserRepository> userRepo)
		:  userRepository(userRepo)
	{}


	Future<User> login( std::string login, std::string pwd )
	{
		auto p = promise<User>();

		userRepository->get_user(login)
		.then([pwd,p](User user)
		{
			cryptoneat::Password pass;
			bool verified = pass.verify(pwd, user.hash() );

			if(!verified) 
			{
				throw LoginEx("error.msg.login.failed");
			}
			p.resolve(user);
		})
		.otherwise(reject(p));

		return p.future();
	}


	Future<User> register_user( 
		const std::string& username,
		const std::string& login,
		const std::string& pwd,
		const std::string& avatar_url
		)
	{
		auto p = promise<User>();

		userRepository->register_user(username, login, pwd, avatar_url)
		.then([p](User user)
		{
			p.resolve(user);
		})		
		.otherwise(reject(p));

		return p.future();		
	}

private:

	std::shared_ptr<UserRepository> userRepository;
};

#endif

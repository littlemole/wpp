#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "repo.h"
#include "cryptoneat/cryptoneat.h"
#include "reproweb/serialization/conneq.h"

using namespace reproweb;

class Controller
{
public:

	Controller( std::shared_ptr<UserRepository> repo)
		: userRepository(repo)
	{}

	async_t<User> get_user( Parameter<Email> email )
	{
		User user = co_await userRepository->get_user(email->value);

		co_return scrub(user);
	}

	async_t<User> login_user( entity<Login> login )
	{
		User user = co_await userRepository->get_user(login->login());

		cryptoneat::Password pass;
		bool verified = pass.verify(login->hash(), user.hash() );

		if(!verified) 
		{
			throw LoginEx("error.msg.login.failed");
		}

		co_return scrub(user);
	}

	async_t<User> register_user( entity<User> user )
	{
		co_await userRepository->register_user(*user);

		co_return scrub(*user);
	}

private:

	std::shared_ptr<UserRepository> userRepository;

	static User scrub(const User& user) 
	{
		return User(user.username(),user.login(),"",user.avatar_url());
	}
};



class Exceptions
{
public:

	Exceptions()
	{}

	void on_http_ex(const HttpEx& ex,Request& req, Response& res)
	{
		ex.render_error(res);
	}	

	void on_std_ex(const std::exception& ex,Request& req, Response& res)
	{
		ServerEx se(ex.what());
		se.render_error(res);
	}
};


#endif
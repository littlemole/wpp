#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "repo.h"
#include "cryptoneat/cryptoneat.h"
#include <reproweb/serialization/web.h>
#include <reproweb/serialization/json.h>
//#include <reproweb/serialization/conneq.h>

using namespace reproweb;

class Controller
{
public:

	Controller( std::shared_ptr<UserMysqlRepository> repo)
		: userRepository(repo)
	{}

	async_json_t<User> get_user( Request& req, Response& res)
	{
		std::string email = Valid::login(req.path.args().get("email"));

		auto p = json_promise<User>();

		userRepository->get_user(email)
		.then([p](User user)
		{
			p.resolve( scrub(user) );
		})
		.otherwise(reject(p));		

		return p.future();
	}

	async_json_t<User> login_user( json_t<Login> login, Request& req, Response& res)
	{
		auto p = json_promise<User>();

		userRepository->get_user(login.value.login())
		.then([p,login](User user)
		{
			cryptoneat::Password pass;
			bool verified = pass.verify(login.value.hash(), user.hash() );

			if(!verified) 
			{
				throw LoginEx("error.msg.login.failed");
			}

			p.resolve( scrub(user) );
		})
		.otherwise(reject(p));	
		
		return p.future();
	}

	async_json_t<User> register_user( json_t<User> user, Request& req, Response& res)
	{
		auto p = json_promise<User>();

		std::cout << JSON::stringify(meta::toJson(*user)) << std::endl;

		userRepository->register_user(user.value)
		.then([p,user]()
		{
			p.resolve(scrub(user.value));
		})
		.otherwise(reject(p));

		return p.future();
	}

private:
	std::shared_ptr<UserMysqlRepository> userRepository;

	static json_t<User> scrub(const User& user) 
	{
		return json_t<User> { User(user.username(),user.login(),"",user.avatar_url()) };
	}
};



class Exceptions
{
public:

	Exceptions()
	{}

	void on_user_not_found_ex(const UserNotFoundEx& ex,Request& req, Response& res)
	{
		render_error(ex,res.not_found());
	}		

	void on_bad_request_ex(const BadRequestEx& ex,Request& req, Response& res)
	{
		render_error(ex,res.bad_request());
	}	

	void on_login_ex(const LoginEx& ex,Request& req, Response& res)
	{
		render_error(ex,res.forbidden());
	}	

	void on_login_already_taken_ex(const LoginAlreadyTakenEx& ex,Request& req, Response& res)
	{
		render_error(ex,res.forbidden());
	}	

	void on_register_ex(const RegistrationEx& ex,Request& req, Response& res)
	{
		render_error(ex,res.bad_request());
	}	

	void on_std_ex(const std::exception& ex,Request& req, Response& res)
	{
		render_error(ex,res.error());
	}

private:

	template<class E>
	void render_error(const E& ex, Response& res)
	{
		std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;

		Json::Value json = meta::exToJson(ex);

		res
		.body(JSON::flatten(json))
		.contentType("application/json")
		.flush();
	}
};


#endif
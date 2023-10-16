#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "model.h"
#include "view.h"
#include "repo.h"
#include <regex>
#include <reproweb/tools/validation.h>
#include <reproweb/serialization/web.h>


class ExampleController
{
public:

	ExampleController( 
		std::shared_ptr<View> view,
		std::shared_ptr<UserRepository> userRepo )
		: view_(view),
		  userRepository(userRepo)
	{}

	void index( Request& req, Response& res)
	{
		auto session = req_session(req);
		if(!session->authenticated)
		{
			throw AuthEx();
		}

		view_->render_index(res,session->data);
	}

	void show_login( Request& req, Response& res)
	{
		view_->render_login(res,"");		
	}

	void show_registration( Request& req, Response& res)
	{
		view_->render_registration(res,"");		
	}

	reproweb::Async login( Request& req, Response& res)
	{
		QueryParams qp(req.body());
		std::string login = qp.get("login");
		std::string pwd   = qp.get("pwd");

		User user = co_await userRepository->get_user(login);
		cryptoneat::Password pass;
		bool verified = pass.verify(pwd, user.hash() );

		std::cout << "valid pwd: " << verified << std::endl;

		if(!verified) throw LoginEx("invalid login/password combination");

		auto session = req_session(req);
		session->authenticated = true;
		session->data = user.toJson();

		view_->redirect_to_index(res);
	}

	void logout( Request& req, Response& res)
	{
		invalidate_session(req);
		view_->redirect_to_login(res);
	}

	reproweb::Async register_user( Request& req, Response& res)
	{
		QueryParams qp(req.body());
		std::string username   = qp.get("username");
		std::string login      = qp.get("login");
		std::string pwd        = qp.get("pwd");
		std::string avatar_url = qp.get("avatar_url");

		User user = co_await userRepository->register_user(username,login,pwd,avatar_url);

		std::cout << "NEW USER SUCESS: " << user.username() << std::endl;
		
		auto session = req_session(req);
		session->authenticated = true;
		session->data = user.toJson();

		view_->redirect_to_index(res);
	}

private:

	std::shared_ptr<View> view_;
	std::shared_ptr<UserRepository> userRepository;
};


class Exceptions
{
public:

	Exceptions(std::shared_ptr<View> view)
		: view_(view)
	{}

	void on_auth_failed(const AuthEx& ex,Request& req, Response& res)
	{
		std::cout << "auth failed: " << ex.what() << std::endl;
		view_->redirect_to_login(res);
	}

	void on_login_failed(const LoginEx& ex,Request& req, Response& res)
	{
		std::cout << "login failed: " <<  ex.what() << std::endl;
		view_->render_login(res,ex.what());
	}

	void on_registration_failed(const RegistrationEx& ex,Request& req, Response& res)
	{
		std::cout << "reg failed: " << ex.what() << std::endl;
		view_->render_registration(res,ex.what());
	}

	void on_std_ex(const std::exception& ex,Request& req, Response& res)
	{
		std::cout << ex.what() << std::endl;
		view_->redirect_to_login(res);
	}

private:

	std::shared_ptr<View> view_;
};


#endif
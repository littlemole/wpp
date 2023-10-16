#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "model.h"
#include "view.h"
#include "valid.h"

#include "cryptoneat/cryptoneat.h"
#include "reproweb/ctrl/controller.h"
#include <reproweb/serialization/web.h>

using namespace reproweb;

class Controller
{
public:

	Controller( std::shared_ptr<Model> model, std::shared_ptr<View> view)
		: model_(model), view_(view)
	{}

	void index( Request& req, Response& res)
	{
		auto session = req_session(req);
		if(!session->authenticated)
		{
			view_->redirect_to_login(req,res);
			return;
		}

		view_->render_index(req,res,session->data);
	}

	void show_login( Request& req, Response& res)
	{
		view_->render_login(req,res,"");
	}

	void show_registration( Request& req, Response& res)
	{
		view_->render_registration(req,res,"");		
	}

	void login( FormParams params, Request& req, Response& res)
	{
		std::string login = Valid::login<LoginEx>(params);
		std::string pwd   = Valid::passwd<LoginEx>(params);

		model_->login(login,pwd)
		.then([this,&req,&res](User user)
		{
			std::cout << "LOGIN SUCCESS" << std::endl;
			auto session = req_session(req);
			session->authenticated = true;

			auto json = meta::toJson(user);
			Json::Value rmvd;
			json.removeMember("pwd",&rmvd);
			session->data = json;

			view_->redirect_to_index(req,res);
		})
		.otherwise([this,&req,&res](const std::exception& ex)
		{
			invalidate_session(req);
			std::cout << "login: " << ex.what() << std::endl;
			view_->render_login(req,res,ex.what());
		});
	}

	void logout( Request& req, Response& res)
	{
		invalidate_session(req);
		view_->redirect_to_login(req,res);
	}

	void register_user( FormParams params, Request& req, Response& res)
	{
		std::string username   = Valid::username(params);
		std::string login      = Valid::login<RegistrationEx>(params);
		std::string pwd        = Valid::passwd<RegistrationEx>(params);
		std::string avatar_url = Valid::avatar(params);

		User user(username,login,pwd,avatar_url);

		model_->register_user(user)
		.then([this,&req,&res](User user)
		{
			auto session = req_session(req);
			session->authenticated = true;
			session->data = meta::toJson(user);

			view_->redirect_to_index(req,res);
		})
		.otherwise([this,&req,&res](const std::exception& ex)
		{
			view_->render_registration(req,res,ex.what());
		});
	}

private:

	std::shared_ptr<Model> model_;
	std::shared_ptr<View> view_;
};


class Exceptions
{
public:

	Exceptions(std::shared_ptr<View> v)
		: view(v)
	{}

	void on_auth_ex(const AuthEx& ex, prio::Request& req, prio::Response& res)
	{
		view->render_login(req,res,ex.what());
	}		

	void on_login_ex(const LoginEx& ex,prio::Request& req, prio::Response& res)
	{
		view->render_login(req,res,ex.what());
	}	

	void on_register_ex(const RegistrationEx& ex,prio::Request& req, prio::Response& res)
	{
		view->render_registration(req,res,ex.what());
	}	

	void on_std_ex(const std::exception& ex,prio::Request& req, prio::Response& res)
	{
		view->render_error(ex,res);
	}

private:

	std::shared_ptr<View> view;
};


#endif
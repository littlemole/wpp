#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "model.h"
#include "view.h"
#include "repo.h"
#include "valid.h"

#include "reproweb/serialization/web.h"

using namespace reproweb;

class Controller
{
public:

	Controller( 
		std::shared_ptr<Users> users,  
		std::shared_ptr<Chat> chat, 
		std::shared_ptr<View> view
	)
		: users_(users), chat_(chat), view_(view)
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

	void login( Request& req, Response& res)
	{
		std::string login;
		std::string pwd;

		try
		{
			QueryParams qp(req.body());
			login = Valid::login<LoginEx>(qp);
			pwd   = Valid::passwd<LoginEx>(qp);
		}
		catch(const std::exception& ex)
		{
			view_->render_login(req,res,ex.what());
			return;
		}		

		users_->login(login,pwd)
		.then([this,&req,&res](User user)
		{
			auto session = req_session(req);
			session->data = user.toJson();
			session->authenticated = true;
			view_->redirect_to_index(req,res);
		})
		.otherwise([this,&req,&res](const std::exception& ex)
		{
			view_->render_login(req,res,ex.what());
		});
	}

	void logout( Request& req, Response& res)
	{
		invalidate_session(req);
		view_->redirect_to_login(req,res);
	}

	Future<> register_user( Request& req, Response& res)
	{
		std::string username;
		std::string login;
		std::string pwd;
		std::string avatar_url;

		try
		{
			QueryParams qp(req.body());
			username   = Valid::username(qp);
			login      = Valid::login<RegistrationEx>(qp);
			pwd        = Valid::passwd<RegistrationEx>(qp);
			avatar_url = Valid::avatar(qp);

    		User user = co_await users_->register_user(username,login,pwd,avatar_url);
//		.then([this,&req,&res](User user)
//		{
			auto session = req_session(req);
			session->data = user.toJson();
			session->authenticated = true;

			view_->redirect_to_index(req,res);
/*		})
		.otherwise([this,&req,&res](const std::exception& ex)
		{
			view_->render_registration(req,res,ex.what());
		});
*/
		}		
		catch(const std::exception& ex)
		{
			view_->render_registration(req,res,ex.what());
			co_return;
		}		

		co_return;
	}

	void postMsg(Request& req, Response& res)
	{
		auto session = req_session(req);
		if(session->authenticated)
		{
			QueryParams qp(req.body());
			auto msg = qp.get("chatMsg");

			Json::Value json(Json::objectValue);
			json["msg"] = msg;

			chat_->sendMsg(session->data,json);

		}
		res.ok().flush();
	}

private:

	std::shared_ptr<Users> users_;
	std::shared_ptr<Chat> chat_;
	std::shared_ptr<View> view_;
};



#endif
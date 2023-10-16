#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "model.h"
#include "view.h"
#include "repo.h"


class Controller
{
public:

	Controller( 
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
			view_->redirect_to_login(res);
			return;
		}

		Json::Value profile = session->data;
		view_->render_index(res,profile);
	}

	void show_login( Request& req, Response& res)
	{
		view_->render_login(res,"");		
	}

	void show_registration( Request& req, Response& res)
	{
		view_->render_registration(res,"");		
	}

	void login( Request& req, Response& res)
	{
		QueryParams qp(req.body());
		std::string login = qp.get("login");
		std::string pwd   = qp.get("pwd");

		userRepository->get_user(login)
		.then( [this,pwd,&req,&res](User user)
		{
			cryptoneat::Password pass;
			bool verified = pass.verify(pwd, user.hash() );

			std::cout << "valid pwd: " << verified << std::endl;

			if(!verified) throw repro::Ex("invalid login/password combination");

			auto session = req_session(req);
			session->authenticated = true;
			session->data = user.toJson();

			view_->redirect_to_index(res);
		})
		.otherwise( [this,&res](const std::exception& ex)
		{
			view_->render_login(res,ex.what());
		});
	}

	void logout( Request& req, Response& res)
	{
		invalidate_session(req);
		view_->redirect_to_login(res);		
	}

	void register_user( Request& req, Response& res)
	{
		QueryParams qp(req.body());
		std::string username   = qp.get("username");
		std::string login      = qp.get("login");
		std::string pwd        = qp.get("pwd");
		std::string avatar_url = qp.get("avatar_url");

		userRepository->register_user(username,login,pwd,avatar_url)
		.then( [this,&req,&res](User user)
		{
			std::cout << "NEW USER SUCESS: " << user.username() << std::endl;

			auto session = req_session(req);
			session->authenticated = true;
			session->data = user.toJson();

			view_->redirect_to_index(res);
		})
		.otherwise( [this,&res](const std::exception& ex)
		{
			view_->render_registration(res,ex.what());
		});		
	}

private:

	std::shared_ptr<View> view_;
	std::shared_ptr<UserRepository> userRepository;
};

#endif
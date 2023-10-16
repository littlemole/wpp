#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "repo.h"

using namespace reproweb;

class Controller
{
public:

	Controller( std::shared_ptr<SessionRepository> repo)
		: sessionRepository(repo)
	{}

	Future<Json::Value> get_session( Parameter<SessionId> params )
	{
		::Session session = co_await sessionRepository->get_user_session(params->sid);

		co_return meta::toJson(session.profile());
	}

	Future<Json::Value> write_session( json_t<User> user)
	{
		::Session session = co_await sessionRepository->write_user_session(*user);

		co_return meta::toJson(session);
	}	

	reproweb::Async remove_session( Parameter<SessionId> params, Response& res)
	{
		co_await sessionRepository->remove_user_session(params->sid);

		res.ok().flush();

		co_return;
	}

private:

	std::shared_ptr<SessionRepository> sessionRepository;
};


class Exceptions
{
public:

	Exceptions()
	{}

	void on_no_session_ex(const NoSessionEx& ex,Request& req, Response& res)
	{
		on_std_ex(ex,req,res);
	}	

	void on_std_ex(const std::exception& ex,Request& req, Response& res)
	{
		std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;

		Json::Value json = meta::exToJson(ex);

		res
		.error()
		.body(JSON::flatten(json))
		.contentType("application/json")
		.flush();
	}		
};


#endif
#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_CONTROLLER_DEFINE_

#include "repo.h"
#include "cryptoneat/cryptoneat.h"
#include <reproweb/serialization/web.h>

using namespace reproweb;

class Controller
{
public:

	Controller( std::shared_ptr<SessionRepository> repo)
		: sessionRepository(repo)
	{}

	Future<Json::Value> get_session( Request& req, Response& res)
	{
		std::string sid = Valid::session_id(req.path.args().get("sid"));

		auto p = promise<Json::Value>();

		sessionRepository->get_user_session(sid)
		.then([p](Session session)
		{
			p.resolve(meta::toJson(session));
		})
		.otherwise([this,p](const std::exception& ex)
		{
			Session session;
			sessionRepository->write_user_session(session)
			.then([p](Session session)
			{
				p.resolve(meta::toJson(session));
			})
			.otherwise(reject(p));
		});

		return p.future();
	}

	Future<Json::Value> write_session( json_t<Session> session, Request& req, Response& res)
	{
		auto p = promise<Json::Value>();

		sessionRepository->write_user_session(*session)
		.then([p](Session session)
		{
			p.resolve(meta::toJson(session));
		})
		.otherwise(reject(p));

		return p.future();	
	}	

	Async remove_session( Request& req, Response& res)
	{
		auto p = promise();

		std::string sid = Valid::session_id(req.path.args().get("sid"));

		sessionRepository->remove_user_session(sid)
		.then([p,&res]()
		{
			res.ok().flush();
			p.resolve();
		})
		.otherwise(reject(p));

		return p.future();	

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
		std::cout << typeid(ex).name() << ":" << ex.what() << std::endl;

		Json::Value json = meta::exToJson(ex);

		res
		.not_found()
		.body(JSON::flatten(json))
		.contentType("application/json")
		.flush();
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
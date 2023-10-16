#include "gtest/gtest.h"

#include "entities.h"

class TestController
{
public:

	TestController() {}

	TestController(std::shared_ptr<Logger> logger)
	: logger_(logger)
	{}
   
	~TestController()
	{
		std::cout << "~TestController" << std::endl;
	}

	static std::string http_root()
	{
		return "/root";
	}

	void postMultipart( prio::MultiParts mp,  prio::Request& req,  prio::Response& res)
	{
		for( auto& p : mp.parts)
		{
			if ( p.headers.content_type() == "text/html")
			{
				res.body( p.body );
				res.ok().contentType("text/html").flush();
				return;
			}
		}
		res.error().body("error").flush();
	}

	void handlerA( prio::Request& req, prio::Response& res)
	{
		logger_->log(req.path.path());
		res.body( "HELO WORL");
		res.ok().flush();
	}

	Async handlerB( prio::Request& req, prio::Response& res) 
	{
		int status = 0;
		std::string header;

		std::cout << "controller B" << std::endl;

		try {

			auto req = reprocurl::async_curl()->url("http://127.0.0.1:8765/path/a");

			reprocurl::CurlEasy::Ptr curl = co_await req->perform();

			status = curl->status();

			res.ok().body(curl->response_body()).header("server", "molws");

	//		res.ok().body("hello").header("server", "molws");

			int i = co_await logger_->test();

			std::cout << i <<  " status: " << status << " server:" << header << std::endl;

			res.flush();

		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			theLoop().exit();
		};
	}

	void handlerSSI( prio::Request& req, prio::Response& res)
	{
		res.contentType("text/html");

		logger_->log(req.path.path());

		char* cwd = getcwd(0,0);
    	std::string path_ = cwd;
    	path_ += "/htdocs";
    	free(cwd); 

        std::regex e ("\\.\\.");
        std::string path = std::regex_replace(req.path.path(),e,"");

        std::regex e2 ("/root");
        path = std::regex_replace(path,e2,"");

        std::string fp = path_ +  path;		

		std::string tmpl = prio::slurp(fp);

		reproweb::SSIResolver::resolve(req,tmpl)
		.then( [&res](std::string s)
		{
			res.body(s);
			res.ok().flush();
		})
		.otherwise([&res](const std::exception& ex)
		{
			res.error().flush();
		});
	}	

	void queryParams( QueryParams qp, /* prio::Request& req,  */ prio::Response& res)
	{
		res.body( qp.get("param") );
		res.ok().flush();
	}
 
	async_json_t<User> getUser()//prio::Request& req, prio::Response& res)
	{
		auto p = json_promise<User>();//promise<json_t<User>>();

		nextTick( [p]()
		{
			User user{ "mike", "littlemole", "secret", { "one", "two", "three"} };
			p.resolve( json_t<User>{user} );
		});

		return p.future();
	}
   
 
	repro::Future<std::string> getParams( Parameter<Input> params)//, prio::Request& req, prio::Response& res)
	{
		auto p = promise<std::string>();

		std::ostringstream oss;
		oss << "======================================" << std::endl;
		oss << params->cookie.str() << std::endl;
		oss << params->sid << std::endl;
		oss << params->filter << std::endl;
		oss << params->id << std::endl;
		oss << "======================================" << std::endl;

		std::string out = oss.str();

		nextTick( [p,params,out]()
		{
			p.resolve( out );
		});

		return p.future();
	}

	repro::Future<json_t<User>> postUser(json_t<User> user)//, prio::Request& req, prio::Response& res)
	{
		auto p = promise<json_t<User>>();

		nextTick( [p,user]()
		{
			p.resolve(user);
		});

		return p.future();
	}


	repro::Future<Json::Value> postUserJson(Json::Value user)//, prio::Request& req, prio::Response& res)
	{
		auto p = promise<Json::Value>();

		nextTick( [p,user]()
		{
			p.resolve(user);
		});

		return p.future();
	}	


	repro::Future<json_t<User>> getUserCoro() //prio::Request& req, prio::Response& res)
	{
		//co_await nextTick();

		User user{ "mike", "littlemole", "secret", { "one", "two", "three"} };
		co_return json_t<User> {user};
	}


	repro::Future<json_t<User>> postUserCoro(json_t<User> user)//, prio::Request& req, prio::Response& res)
	{
		//co_await nextTick();
		std::cout << JSON::stringify(meta::toJson(*user)) << std::endl;
		co_return user;
	}


	repro::Future<Json::Value> postUserJsonCoro(Json::Value user)//, prio::Request& req, prio::Response& res)
	{
		//co_await nextTick();

		co_return user;
	}


private:

	std::shared_ptr<Logger> logger_;
};


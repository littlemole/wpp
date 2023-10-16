#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/ctrl/redissession.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
  
#include "model.h"
#include "view.h"
#include "repo.h"
#include "controller.h"


#define TO_STR_HELPER(x) #x
#define TO_STR(x) TO_STR_HELPER(x)

class AppConfig : public Config
{
public:
	AppConfig()
	  : Config("config.json")
	{
		const char* redis = getenv("REDIS_HOST");
		if(redis)
		{
			std::ostringstream oss;
			oss << "redis://" << redis << ":6379";

			std::cout << "REDIS_URL: " << oss.str() << std::endl;

			get("redis") = oss.str();
		}
		json()["version"] = TO_STR(VERSION);
	}
};

struct SessionPool : public reproredis::RedisPool
{
	SessionPool(std::shared_ptr<Config> config) 
	  : RedisPool(config->getString("redis")) 
	{}
};

struct UserPool : public reprosqlite::SqlitePool
{
	UserPool(std::shared_ptr<Config> config) 
	  : SqlitePool(config->getString("sqlite")) 
	{}
};




int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

	WebApplicationContext ctx {

		GET  ( "/",				&ExampleController::index),
		GET  ( "/logout",		&ExampleController::logout),
		GET  ( "/login",		&ExampleController::show_login),
		GET  ( "/register",		&ExampleController::show_registration),
		POST ( "/login",		&ExampleController::login),
		POST ( "/register",		&ExampleController::register_user),

		session_filter<>("GET|POST", "^((?!(/css)|(/img)|(/js)|(/inc)).)*$", 10),

		ex_handler(&Exceptions::on_auth_failed),
		ex_handler(&Exceptions::on_login_failed),
		ex_handler(&Exceptions::on_registration_failed),
		ex_handler(&Exceptions::on_std_ex),

#ifndef _WIN32
		static_content("/htdocs/","/etc/mime.types"),
#else
		static_content("/htdocs/","mime.types"),
#endif

		singleton<AppConfig()>(),
		singleton<SessionPool(AppConfig)>(),
		singleton<UserPool(AppConfig)>(),

		singleton<UserRepository(UserPool)>(),

		singleton<View(AppConfig)>(),
		singleton<ExampleController(View,UserRepository)>(),

		singleton<Exceptions(View)>(),

		singleton<RedisSessionProvider(SessionPool)>(),
		singleton<SessionFilter(RedisSessionProvider)>()
	};	

	Http2SslCtx sslCtx;
	sslCtx.load_cert_pem("pem/server.pem");
	//sslCtx.enableHttp2();

	WebServer server(ctx);
	server.listen(sslCtx,9876);
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

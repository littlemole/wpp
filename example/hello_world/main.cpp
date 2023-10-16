#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/ctrl/session.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
      
#include "model.h"
#include "view.h"
#include "repo.h"
#include "controller.h"

using namespace diy;  
using namespace prio;
using namespace reproweb;

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

			get("redis") = oss.str();
		}

		json()["version"] = TO_STR(VERSION);
	}
};


struct UserPool : public reprosqlite::SqlitePool
{
	UserPool(std::shared_ptr<Config> config) 
	  : SqlitePool(config->getString("sqlite")) 
	{}
};

void server()
{
	WebApplicationContext ctx{

		GET("/",						&Controller::index),
		GET("/logout",			&Controller::logout),
		GET("/login",				&Controller::show_login),
		GET("/register",		&Controller::show_registration),
		POST("/login",			&Controller::login),
		POST("/register",		&Controller::register_user),

		singleton<AppConfig()>(),
		singleton<UserPool(AppConfig)>(),
		singleton<UserRepository(UserPool)>(),

		singleton<View(AppConfig)>(),
		singleton<Controller(View,UserRepository)>(),

		singleton<SessionFilter(MemorySessionProvider)>()		
	};


	WebServer webserver(ctx);
	webserver.configure<AppConfig>();
	webserver.session<SessionFilter>();
	webserver.listen();

	theLoop().run();
}

int main(int argc, char **argv)
{
	try
	{
		std::cout << TO_STR(VERSION) << std::endl;

		prio::Libraries<prio::EventLoop, cryptoneat::SSLUser> init;

		server();
	}
	catch (const std::exception& ex)
	{
		std::cout << typeid(ex).name() << ": " << ex.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "abort with unknown exception." << std::endl;
	}

	MOL_TEST_PRINT_CNTS();	
	std::cout << "exit" << std::endl;
    return 0;
}

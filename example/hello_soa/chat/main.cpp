#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/view/i18n.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
   
#include "model.h"
#include "valid.h"
#include "view.h"
#include "service.h"
#include "controller.h"
#include "ws.h"
#include <curl/curl.h>

using namespace diy;  
using namespace prio;
using namespace reproweb;



class SessionServiceProvider : public  SessionProvider
{
public:

	SessionServiceProvider(std::shared_ptr<SessionService> service)
		: sessionService_(service)
	{}


    virtual repro::Future<reproweb::Session> get_session( std::string sid)
	{
		return sessionService_->get_user_session(sid);
	}

    virtual repro::Future<> set_session(reproweb::Session session)
	{
        std::cout << "write SESSION: " << JSON::stringify(meta::toJson(session)) << std::endl;

		return sessionService_->write_user_session(session);
	} 

    virtual repro::Future<> remove_user_session(reproweb::Session session)
	{
		return sessionService_->remove_user_session(session.sid);
	}

private:

	std::shared_ptr<SessionService> sessionService_;
};


int main(int argc, char **argv)
{
	prio::Libraries<
		prio::EventLoop, 
		cryptoneat::SSLUser, 
		reprocurl::ReproCurl> 
	init;

	WebApplicationContext ctx {

		GET  ( "/",				&Controller::index),
		GET  ( "/logout",		&Controller::logout),
		GET  ( "/login",		&Controller::show_login),
		GET  ( "/register",		&Controller::show_registration),
		POST ( "/login",		&Controller::login),
		POST ( "/register",		&Controller::register_user),

		ws_controller<WebSocketController> ("/ws"),

		session_filter<SessionFilter>("(GET)|(POST)" , "^((?!(/css)|(/img)|(/js)|(/inc)).)*$" , 10),

		ex_handler(&Exceptions::on_auth_ex),
		ex_handler(&Exceptions::on_login_ex),
		ex_handler(&Exceptions::on_register_ex),
		ex_handler(&Exceptions::on_std_ex),

		singleton<AppConfig()>(),

		singleton<UserService(AppConfig)>(),
		singleton<Model(UserService)>(),
		singleton<View(AppConfig,TplStore,I18N)>(),
		singleton<Controller(Model,View)>(),

		singleton<Redis(AppConfig)>(),
		singleton<RedisBus(Redis)>(),
		singleton<WebSocketController(SessionService,RedisBus)>(),

		singleton<Exceptions(View)>(),

		singleton<SessionService(AppConfig)>(),
		singleton<SessionServiceProvider(SessionService)>(),
		singleton<SessionFilter(SessionServiceProvider)>()
	};	
 

	WebServer server(ctx);
	server.configure<AppConfig>();
	server.listen();
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

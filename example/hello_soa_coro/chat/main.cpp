#include "reprocpp/test.h"
#include "reproweb/web_webserver.h"
  
#include "model.h"
#include "view.h"
#include "service.h"
#include "controller.h"
#include "ws.h"

using namespace diy;  
using namespace prio;
using namespace reproweb;


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

		ex_handler(&Exceptions::on_auth_ex),
		ex_handler(&Exceptions::on_login_ex),
		ex_handler(&Exceptions::on_register_ex),
		ex_handler(&Exceptions::on_std_ex),

		singleton<AppConfig()>(),

		singleton<SessionService(AppConfig)>(),
		singleton<UserService(AppConfig)>(),

		singleton<Model(SessionService,UserService)>(),
		singleton<View(AppConfig,TplStore,I18N)>(),
		singleton<Controller(Model,View)>(),

		singleton<WebSocketController(SessionService,EventBus)>(),

		singleton<Exceptions(View)>()
	};	
 
	WebServer server(ctx);
	server.configure<AppConfig>();
	server.listen();
      
	theLoop().run(); 
	 
	MOL_TEST_PRINT_CNTS();	
    return 0;
}

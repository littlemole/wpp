#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/view/i18n.h"
#include "reproweb/view/tpl.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
     
#include "model.h"
#include "valid.h"
#include "view.h"
#include "repo.h"
#include "controller.h"
#include "ws.h"
  
using namespace diy;  
using namespace prio;
using namespace reproweb;



int main(int /*argc*/, char** /*argv*/ )
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

	WebApplicationContext ctx {

		GET  ( "/",				&Controller::index),
		GET  ( "/logout",		&Controller::logout),
		GET  ( "/login",		&Controller::show_login),
		GET  ( "/register",		&Controller::show_registration),
		POST ( "/login",		&Controller::login),
		POST ( "/register",		&Controller::register_user),

		ws_controller<WebSocketController> ("/ws"),

		singleton<AppConfig()>(),
		singleton<UserPool(AppConfig)>(),

		singleton<UserRepository(UserPool)>(),

		singleton<Users(UserRepository)>(),
		singleton<View(AppConfig,TplStore,I18N)>(),
		singleton<Controller(Users,View)>(),

		singleton<EventBus()>(),
		singleton<WebSocketController(MemorySessionProvider,EventBus)>(),

		singleton<SessionFilter(MemorySessionProvider)>()
	};	


	WebServer server(ctx);
	server.configure<AppConfig>();
	server.session<SessionFilter>();
	server.filter("auth",&Controller::authFilter);
	server.listen();
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

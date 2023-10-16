#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/view/i18n.h"
#include "reproweb/web_webserver.h"
#include <signal.h>

#include "metrics.h"
#include "model.h"
#include "view.h"
#include "repo.h"
#include "controller.h"
#include "ws.h"

using namespace diy;  
using namespace prio;
using namespace reproweb;



int main(int argc, char **argv)
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

		session_filter<SessionFilter>("(GET|POST)", "^((?!(/css)|(/img)|(/js)|(/inc)).)*$",10),

		i18n_props("/locale/properties", {"en", "de"} ),

		view_templates("/view/"),

#ifndef _WIN32
		static_content("/htdocs/","/etc/mime.types"),
#else
		static_content("/htdocs/","mime.types"),
#endif

		ex_handler(&Exceptions::on_auth_failed),
		ex_handler(&Exceptions::on_login_failed),
		ex_handler(&Exceptions::on_registration_failed),
		ex_handler(&Exceptions::on_std_ex),

		singleton<AppConfig()>(),
		singleton<UserPool(AppConfig)>(),

		singleton<UserRepository(UserPool)>(),

		singleton<Model(UserRepository)>(),
		singleton<View(AppConfig,TplStore,I18N)>(),
		singleton<Controller(Prometheus::Collector,Model,View)>(),

		singleton<EventBus()>(),
		singleton<WebSocketController(MemorySessionProvider,EventBus)>(),

		singleton<Exceptions(View)>(),

		singleton<SessionFilter(MemorySessionProvider)>(),

		singleton<Prometheus::Collector()>(),
		singleton<Prometheus::Controller(Prometheus::Collector)>(),

		GET( "/metrics", &Prometheus::Controller::metrics)
	};	

	Http2SslCtx sslCtx;
	sslCtx.load_cert_pem("pem/server.pem");

	sslCtx.enableHttp2();

	WebServer server(ctx);
	server.listen(sslCtx,9876);

	WebServer prometheus(ctx);
	prometheus.listen(9001);
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

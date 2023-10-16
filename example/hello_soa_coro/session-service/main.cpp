#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
  
#include "controller.h"

using namespace diy;  

int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

	WebApplicationContext ctx {

		GET  ( "/session/{sid}",	&Controller::get_session),
		DEL  ( "/session/{sid}",	&Controller::remove_session),
		POST ( "/session",			&Controller::write_session),

		ex_handler(&Exceptions::on_std_ex),
		ex_handler(&Exceptions::on_no_session_ex),

		singleton<AppConfig()>(),
		singleton<SessionPool(AppConfig)>(),
		singleton<SessionRepository(SessionPool)>(),
		singleton<Controller(SessionRepository)>()
	};	

	WebServer server(ctx);
	server.configure<AppConfig>();
	server.listen();	
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

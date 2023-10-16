#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
  
#include "repo.h"
#include "controller.h"

using namespace diy;  
using namespace prio;
using namespace reproweb;


int main(int argc, char **argv)
{
	prio::Libraries<
		repromysql::MySQL,
		prio::EventLoop, 
		cryptoneat::SSLUser> 
	init;

	WebApplicationContext ctx {

		GET  ( "/user/{email}",		&Controller::get_user),
		POST ( "/register",			&Controller::register_user),
		POST ( "/login",			&Controller::login_user),

		ex_handler(&Exceptions::on_user_not_found_ex),
		ex_handler(&Exceptions::on_login_ex),
		ex_handler(&Exceptions::on_login_already_taken_ex),
		ex_handler(&Exceptions::on_bad_request_ex),
		ex_handler(&Exceptions::on_register_ex),
		ex_handler(&Exceptions::on_std_ex),

		singleton<AppConfig()>(),
		singleton<UserMysqlPool(AppConfig)>(),
		singleton<UserMysqlRepository(UserMysqlPool)>(),

		singleton<Controller(UserMysqlRepository)>(),

		singleton<Exceptions()>()
	};	

	std::string cert = diy::inject<AppConfig>(ctx)->getString("cert");

	Http2SslCtx sslCtx;
	sslCtx.load_cert_pem(cert);
	sslCtx.set_client_ca("pem/ca.crt");
	//sslCtx.enableHttp2();

	WebServer server(ctx);
	server.listen(sslCtx,9877);
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

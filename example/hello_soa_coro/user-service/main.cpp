#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
  
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

		ex_handler(&Exceptions::on_http_ex),
		ex_handler(&Exceptions::on_std_ex),

		singleton<UserPool(AppConfig)>(),
		singleton<UserRepository(UserPool)>(),

		singleton<Controller(UserRepository)>()
	};	

	std::string cert = diy::inject<AppConfig>(ctx)->getString("cert");

	Http2SslCtx sslCtx;
	sslCtx.load_cert_pem(cert);
	//sslCtx.enableHttp2();

	WebServer server(ctx);
	server.listen(sslCtx,9877);
     
	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

#include <reprocpp/after.h>
#include <signal.h>
#include <priocpp/api.h>
#include <priocpp/task.h>
#include <cryptoneat/cryptoneat.h>
#include "test.h"

using namespace prio;


struct client_handler
{
	client_handler(Connection::Ptr c)
	: con(c)
	{}

	void handle()
	{
		con->read()
		.then( [this](Connection::Ptr client,std::string data)
		{
			std::cout << "server read:" << data << std::endl;
			if( data.substr(0,4) == "quit")
			{
				throw repro::Ex("quit");
			}
			return con->write(data);
		})
		.then([this](Connection::Ptr client)
		{
			handle();
		})
		.otherwise([this](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			delete this;
		});
	}

	Connection::Ptr con;
};


int main(int argc, char **argv) {

	prio::Libraries<prio::EventLoop, cryptoneat::SSLUser> init;

	{
		prio::SslCtx ctx;
		ctx.load_cert_pem("pem/server.pem");

		Listener listener(ctx);

		prio::signal(SIGINT).then( [&listener](int s)
		{
			listener.cancel();
			nextTick([]()
			{
				theLoop().exit(); 
			}); 
		});

		listener.bind(9876)
		.then([](Connection::Ptr client)
		{
			(new client_handler(client))->handle();
		});

		theLoop().run();
	}

	MOL_TEST_PRINT_CNTS();

    return 0;
}

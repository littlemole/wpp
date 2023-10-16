#include "reprocpp/test.h"
#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
#include "reproweb/ws/websocket.h"
#include "cryptoneat/cryptoneat.h"
#include <signal.h>
#include <iostream>
  
using namespace diy;  
using namespace prio;
using namespace reproweb;


prio::IO io;

std::string buffer;
WsConnection::Ptr ws;

void read_stdin()
{
	io.onRead(0)
	.then([]()
	{
		char buf[1024];
		int len = read(0,buf,1024);
		//write(1,buf,len);
		if(buf[0] == '.' && buf[1] == '\n')
		{
			ws->send(0x01,buffer);
			buffer = "";
		}
		else
		{
			buffer.append(std::string(buf,len));
		}
		read_stdin();
	});
}

int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop, cryptoneat::SSLUser> init;

	Http2SslCtx sslCtx;
	//sslCtx.enableHttp2();

	timeout( [&sslCtx,&ws] () 
	{
		ws = WsConnection::create(sslCtx);
		ws->connect("wss://echo.websocket.org:443/?encoding=text")
		.then( [](WsConnection::Ptr ws)
		{
			std::cout << "created" << std::endl;

			ws->onConnect([](WsConnection::Ptr ws)
			{
				std::cout << "connected" << std::endl;
			});

			ws->onClose([](WsConnection::Ptr ws)
			{
				std::cout << "close" << std::endl;
				theLoop().exit(); 
			});

			ws->onMsg([](WsConnection::Ptr ws, std::string s)
			{
				std::cout << s << std::endl;
			});
			ws->send(0x01,"huhu");
		});
	},0,500);

	signal(SIGINT).then([](int s) 
	{ 
		theLoop().exit(); 
	});

	signal(SIGQUIT).then([&ws](int s) 
	{
		ws->send(0x01,buffer);
		buffer = "";
	});


    read_stdin();

	theLoop().run();

	MOL_TEST_PRINT_CNTS();	
    return 0;
}

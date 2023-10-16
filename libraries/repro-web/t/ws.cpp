#include "webtest.h"

#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif


class BasicWsTest : public ::testing::Test {
 protected:

  static void SetUpTestCase() {

	  //theLoop().signal(SIGPIPE).then([](int s){});

  }

  virtual void SetUp() {
	 // MOL_TEST_PRINT_CNTS();
  }
 
  virtual void TearDown() {
	 // MOL_TEST_PRINT_CNTS();
  }
}; // end test setup


singleton<Logger()> LoggerComponent;
singleton<TestController(Logger)> TestControllerComponent;


#include <reproweb/ws/ws.h>
#include <fcntl.h>
#include <signal.h>


class WebSocketController
{
public:

	WebSocketController()
	{}

    void onConnect(WsConnection::Ptr ws)
    {
    	std::cout << "ws on connect" << std::endl;
    };

    void onClose(WsConnection::Ptr ws)
    {
    	std::cout << "ws on close" << std::endl;
    };

    void onMsg(WsConnection::Ptr ws, const std::string& data)
	{
    	std::cout << "ws: " << data << std::endl;
    	ws->send(0x01,data);
	};

};

provider<WebSocketController()> WebSocketControllerComponent;


TEST_F(BasicWsTest, SimpleHttp) {

	std::string result;


	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,
		WebSocketControllerComponent,

		ws_controller<WebSocketController>( "/ws"),

		GET ("/path/a",&TestController::handlerA),

		POST("/path/b",&TestController::handlerB),
	};	

	{

		reproweb::WebServer server(ctx);


		//ws_controller<WebSocketController> ws(inject<FrontController>(ctx),"/ws");

		WsConnection::Ptr client= WsConnection::create();

		nextTick([&client,&server]()
		{
			client->connect("ws://localhost:8765/ws")
			.then( [&server](WsConnection::Ptr ws)
			{
				std::cout << "connected" << std::endl;
				ws->onMsg([&server](WsConnection::Ptr ws, const std::string& data)
				{
					std::cout << "ws client: " << data << std::endl;
					//ws->connection()->shutdown();
					ws->close();
					//ws->connection()->close();
					//ws->dispose();
					//loop.exit();
					timeout([&server]()
					{
						std::cout << "TIMEOUT" << std::endl;
						server.shutdown();
						timeout([]()
						{
							theLoop().exit();
						},0,200);
					},0,200);
				});
				ws->onClose( [](WsConnection::Ptr ws) 
				{
					std::cout << "client ws on close" << std::endl;
				});
				ws->send(0x01,"HELO");
			});

		});

		server.listen(8765);
		theLoop().run();
	}

	EXPECT_EQ(1,1);
	MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicWsTest, SimpleHttps) {

	std::string result;


	WebApplicationContext ctx {

		ws_controller<WebSocketController>( "/ws"),

		GET ("/path/a",&TestController::handlerA),

		POST("/path/b",&TestController::handlerB),
		LoggerComponent,
		TestControllerComponent,
		WebSocketControllerComponent
	};

	{
		prio::SslCtx server_ctx;
		server_ctx.load_cert_pem("pem/server.pem");
		prio::SslCtx client_ctx;
		client_ctx.set_ca_path("pem/ca.crt");

		reproweb::WebServer server(ctx);


		//ws_controller<WebSocketController> ws(inject<FrontController>(ctx),"/ws");

		WsConnection::Ptr client;

		timeout([&client_ctx,&client,&server]()
		{
			client= WsConnection::create(client_ctx);
			client->connect("wss://localhost:8766/ws")
			.then( [&server](WsConnection::Ptr ws)
			{
				std::cout << "connected" << std::endl;
				ws->onMsg([&server](WsConnection::Ptr ws, const std::string& data)
				{
					std::cout << "ws client: " << data << std::endl;
					//ws->connection()->shutdown();
					ws->close();
					//ws->connection()->close();
					//ws->dispose();
			    	//loop.exit();
					timeout([&server]()
					{
						std::cout << "TIMEOUT" << std::endl;
						server.shutdown();
						timeout([]()
						{
							theLoop().exit();
						}, 0, 200);
						
					},0,200);
				});

				ws->send(0x01,"HELO");
			});

		},0,500);

		server.listen(server_ctx,8766);
		theLoop().run();

	}

	EXPECT_EQ(1,1);
	MOL_TEST_ASSERT_CNTS(0,0);
}


int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

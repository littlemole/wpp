#include "webtest.h"
#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif

 

class BasicSSLTest : public ::testing::Test {
 protected:

  static void SetUpTestCase() 
  {

  }

  virtual void SetUp() 
  {
	 // MOL_TEST_PRINT_CNTS();
  }
 
  virtual void TearDown() 
  {
	 // MOL_TEST_PRINT_CNTS();
  }
}; // end test setup

 
singleton<Logger()> LoggerComponent;
singleton<TestController(Logger)> TestControllerComponent;


TEST_F(BasicSSLTest, SimpleSSL) {

	std::string result;


	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::handlerA)

		,
		POST("/path/b",&TestController::handlerB)
	};

	{
		prio::SslCtx server_ctx;
		server_ctx.load_cert_pem("pem/server.pem");

		prio::SslCtx client_ctx;
		client_ctx.set_ca_path("pem/ca.crt");

		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&client_ctx,&server]()
		{
			HttpClient::url(client_ctx,"https://localhost:8765/root/path/a")
			->fetch()
			.then([&result,&server](prio::Response& res)
			{
				result = res.body();

				timeout([&server]()
				{
					//theLoop().exit();
					server.shutdown();
					theLoop().exit();
				},1,0);
			});
		});
		
		server.listen(server_ctx,8765);
		theLoop().run();

	}
    EXPECT_EQ("HELO WORL",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}



int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

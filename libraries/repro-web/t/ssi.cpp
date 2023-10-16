#include "webtest.h"

#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif


class BasicSSITest : public ::testing::Test {
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




TEST_F(BasicSSITest, SplitSSI) 
{
	std::string ssisrc = 
	"<html><head></head><body>"
	"<table><tr><td>"
	"<!--#include virtual='/inc/header.html' -->"
	"</td></tr><tr><td>"
	"<!--#include virtual='/inc/main.html' -->"
	"</td></tr><tr><td>"
	"<!--#include virtual='/inc/footer.html' -->"
	"</td></tr><tr><td>"
	"</td></tr></table>"
	"</body></html>";

	std::regex r("<!--#include +virtual=(?:[\"'])([^\"']+)(?:[\"']) *-->");
	std::smatch match;

	std::string::const_iterator start = ssisrc.begin();
    std::string::const_iterator end   = ssisrc.end();    

	std::vector<std::string> result;
    
    while (std::regex_search (start,end,match,r)) 
    {
        if ( match.size() > 1 )
        {
			std::cout << std::string(start,match[0].first) << std::endl;
			std::cout << "inc: " << match[1] << std::endl;
            result.push_back(match[1]);
        }
        start = match[0].second;
    }
	std::cout << std::string(start,end) << std::endl;
	EXPECT_EQ(3,result.size());
}

TEST_F(BasicSSITest, ResolveSSI) 
{
	std::string ssisrc = 
	"<html><head></head><body>"
	"<table><tr><td>"
	"<!--#include virtual='/root/path/a' -->"
	"</td></tr><tr><td>"
	"<!--#include virtual='/root/path/a' -->"
	"</td></tr><tr><td>"
	"<!--#include virtual='/root/path/a' -->"
	"</td></tr><tr><td>"
	"</td></tr></table>"
	"</body></html>";

	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::handlerA)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&ctx,ssisrc]()
		{

			auto fc = inject<FrontController>(ctx);

			Request req;
			req.attributes.set<std::shared_ptr<diy::Context>>("ctx",std::make_shared<Context>(&ctx));
			req.path.method("GET");

			return reproweb::SSIResolver::resolve(req,ssisrc);
		})
		.then( [&result,&server](std::string s)
		{
				result = s;
				server.shutdown();
				theLoop().exit();
		})
		.otherwise([&server](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			server.shutdown();
			theLoop().exit();
		});

		server.listen(8765);
		theLoop().run();
	}
    EXPECT_EQ("<html><head></head><body><table><tr><td>HELO WORL</td></tr><tr><td>HELO WORL</td></tr><tr><td>HELO WORL</td></tr><tr><td></td></tr></table></body></html>",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicSSITest, handleSSI) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::handlerA),
		GET ("/index.shtml",&TestController::handlerSSI)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/index.shtml")
			->fetch()
			.then([&result,&server](prio::Response& res)
			{
				result = res.body();
				server.shutdown();
				theLoop().exit();
			})
			.otherwise([&server](const std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
				server.shutdown();
				theLoop().exit();
			});
		});

		server.listen(8765);
		theLoop().run();
	}
    EXPECT_EQ("<html><head></head><body>\n<table><tr><td>\nHELO WORL\n</td></tr><tr><td>\nHELO WORL\n</td></tr><tr><td>\nHELO WORL\n</td></tr><tr><td>\n</td></tr></table>\n</body></html>\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicSSITest, autoHandleSSI) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::handlerA),
		ssi_content("/htdocs", "/.*\\.shtml")
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/index.shtml")
			->fetch()
			.then([&result,&server](prio::Response& res)
			{
				result = res.body();
				server.shutdown();
				theLoop().exit();
			})
			.otherwise([&server](const std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
				server.shutdown();
				theLoop().exit();
			});
		});

		server.listen(8765);
		theLoop().run();
	}
    EXPECT_EQ("<html><head></head><body>\n<table><tr><td>\nHELO WORL\n</td></tr><tr><td>\nHELO WORL\n</td></tr><tr><td>\nHELO WORL\n</td></tr><tr><td>\n</td></tr></table>\n</body></html>\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}


  

TEST_F(BasicSSITest, SimpleInclude) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::handlerA)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&ctx]()
		{
			auto fc = inject<FrontController>(ctx);

			Request req;
			req.path.method("GET");

			return fc->include(req,"/root/path/a");
		})
		.then( [&result,&server](std::string s)
		{
				result = s;
				server.shutdown();
				theLoop().exit();
		})
		.otherwise([&server](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			server.shutdown();
			theLoop().exit();
		});

		server.listen(8765);
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

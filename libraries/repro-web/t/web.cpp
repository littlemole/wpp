#include "webtest.h"

#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif

 
 class BasicWebTest : public ::testing::Test {
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


TEST_F(BasicWebTest, SimpleDI) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::handlerA)
		,
		POST("/path/b",&TestController::handlerB)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
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
    EXPECT_EQ("HELO WORL",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}





repro::Future<> coroutine_example(reproweb::WebServer& server, std::string& result);

TEST_F(BasicWebTest, coroutine)
{
	std::string result;

	WebApplicationContext ctx {

		GET ("/path/a",&TestController::handlerA),

		POST("/path/b",&TestController::handlerB),
		LoggerComponent,
		TestControllerComponent
	};

	{
		reproweb::WebServer server(ctx);

		coroutine_example(server,result)
		.then([](){})
		.otherwise([](const std::exception& ex){});

		server.listen(8765);
		theLoop().run();

	}
	EXPECT_EQ("molws", result);
	MOL_TEST_ASSERT_CNTS(0, 0);

}

repro::Future<> coroutine_example(reproweb::WebServer& server, std::string& result)
{
	try
	{
		(void) co_await nextTick();

		auto post = reprocurl::async_curl()->url("http://127.0.0.1:8765/root/path/b")->method("POST")->data("dummy");

		reprocurl::CurlEasy::Ptr curl = co_await post->perform();

		result = curl->response_header("server");
		server.shutdown();

		timeout( []() {
			theLoop().exit();
			reprocurl::curl_multi().dispose();
		},1,0);
	}
	catch(const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		server.shutdown();
		theLoop().exit();
		reprocurl::curl_multi().dispose();
	}
}



TEST_F(BasicWebTest, SimpleRest) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::getUser)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
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
    EXPECT_EQ("{\"user\":{\"login\":\"littlemole\",\"pwd\":\"secret\",\"tags\":[\"one\",\"two\",\"three\"],\"username\":\"mike\"}}",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

 
const char* multipart = "-----------------------------9051914041544843365972754266\r\n"
"Content-Disposition: form-data; name=\"text\"\r\n"
"\r\n"
"text default\r\n"
"-----------------------------9051914041544843365972754266\r\n"
"Content-Disposition: form-data; name=\"file1\"; filename=\"a.txt\"\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"Content of a.txt.\r\n"
"\r\n"
"-----------------------------9051914041544843365972754266\r\n"
"Content-Disposition: form-data; name=\"file2\"; filename=\"a.html\"\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<!DOCTYPE html><title>Content of a.html.</title>\r\n"
"\r\n"
"-----------------------------9051914041544843365972754266--\r\n\r\n";


TEST_F(BasicWebTest, SimpleMultipart) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		POST ("/path/a",&TestController::postMultipart)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
			->content_type("multipart/form-data;boundary=\"---------------------------9051914041544843365972754266\"")
			->POST(multipart)
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
    EXPECT_EQ("<!DOCTYPE html><title>Content of a.html.</title>\r\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicWebTest, SimpleRestParams) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/{id}",&TestController::getParams)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a?filter=123456789")
			->header("Cookie", prio::Cookie("sid","987654321").path("/").domain("localhost").secure().maxAge(100).str())
			->header("Accept-Language", "de-DE")
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
    EXPECT_EQ("======================================\nsid=987654321;max-Age=100;domain=localhost;path=/;secure\n987654321\n123456789\na\n======================================\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicWebTest, SimpleRestQueryParams) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::queryParams)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a?param=test")
			->GET()
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
    EXPECT_EQ("test",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}



TEST_F(BasicWebTest, SimpleRestPost) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		POST ("/path/a",&TestController::postUser)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
			->POST("{\"user\":{\"login\" : \"littlemole\",\"pwd\" : \"secret\",\"tags\" : [\"one\",\"two\",\"three\"],\"username\" : \"mike\"\n}}")
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
    EXPECT_EQ("{\"user\":{\"login\":\"littlemole\",\"pwd\":\"secret\",\"tags\":[\"one\",\"two\",\"three\"],\"username\":\"mike\"}}",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicWebTest, SimpleRestPost_invalid) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		POST ("/path/a",&TestController::postUser)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
			->POST("{\"user\":{\"login\" : \"<littlemole>\",\"pwd\" : \"secret\",\"tags\" : [\"one\",\"two\",\"three\"],\"username\" : \"mike\"\n}}")
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
    EXPECT_EQ("invalid login",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicWebTest, SimpleRestPostJson) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		POST ("/path/a",&TestController::postUserJson)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
			->POST("{\"login\" : \"littlemole\",\"pwd\" : \"secret\",\"tags\" : [\"one\",\"two\",\"three\"],\"username\" : \"mike\"\n}")
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
    EXPECT_EQ("{\"login\":\"littlemole\",\"pwd\":\"secret\",\"tags\":[\"one\",\"two\",\"three\"],\"username\":\"mike\"}",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}




TEST_F(BasicWebTest, SimpleRestCoro) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		GET ("/path/a",&TestController::getUserCoro)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
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
    EXPECT_EQ("{\"user\":{\"login\":\"littlemole\",\"pwd\":\"secret\",\"tags\":[\"one\",\"two\",\"three\"],\"username\":\"mike\"}}",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicWebTest, SimpleRestPostCoro) 
{
	std::string result;

	std::string userJson("{\"user\":{\"login\" : \"littlemole\",\"pwd\" : \"secret\",\"tags\" : [\"one\",\"two\",\"three\"],\"username\" : \"mike\"\n}}");

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		POST ("/path/a",&TestController::postUserCoro)
	};

	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server,&userJson]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
			->POST(userJson)
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
    EXPECT_EQ("{\"user\":{\"login\":\"littlemole\",\"pwd\":\"secret\",\"tags\":[\"one\",\"two\",\"three\"],\"username\":\"mike\"}}",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicWebTest, SimpleRestPostJsonCoro) 
{
	std::string result;

	WebApplicationContext ctx {

		LoggerComponent,
		TestControllerComponent,

		POST ("/path/a",&TestController::postUserJsonCoro)
	};
 
	{
		reproweb::WebServer server(ctx);

		nextTick()
		.then( [&result,&server]()
		{
			HttpClient::url("http://localhost:8765/root/path/a")
			->POST("{\"login\" : \"littlemole\",\"pwd\" : \"secret\",\"tags\" : [\"one\",\"two\",\"three\"],\"username\" : \"mike\"\n}")
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
    EXPECT_EQ("{\"login\":\"littlemole\",\"pwd\":\"secret\",\"tags\":[\"one\",\"two\",\"three\"],\"username\":\"mike\"}",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}



int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

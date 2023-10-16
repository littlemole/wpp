#include "gtest/gtest.h"
#include "reprocpp/after.h"
#include "reprocpp/test.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include <reprocurl/api.h>
#include <signal.h>
#include <openssl/pem.h>

#ifdef _WIN32
#include <openssl/applink.c>
#endif


using namespace repro;
using namespace prio;
using namespace reprocurl;


class API2Test : public ::testing::Test {
 protected:

  virtual void SetUp() 
  {}

  virtual void TearDown() 
  {}
}; // end test setup




TEST_F(API2Test, SimpleHttp2) 
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	int status = 200;
	{
	        auto req1 = request(prio::Url("https://www.amazon.de/"));
	        auto req2 = request(prio::Url("https://www.amazon.de/"));

		std::vector<request> requests {req1,req2};

		fetch_all(requests)
		.then([&status](std::vector<response> responses)
		{
			for ( auto& r : responses)
			{
				if(status==200) status = r.status();
			}

			theLoop().exit();
		})
		.otherwise([](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});

		theLoop().run();

	        // can only call that once
		curl_multi().dispose();
	}

	EXPECT_EQ(200,status);
	MOL_TEST_ASSERT_CNTS(0, 0);
}

int main(int argc, char **argv) 
{

	prio::Libraries<prio::EventLoop, ReproCurl> init;

	OpenSSL_add_all_algorithms();



    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

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


class APITest : public ::testing::Test {
 protected:

  virtual void SetUp() 
  {}

  virtual void TearDown() 
  {}
}; // end test setup




TEST_F(APITest, SimpleHttp) 
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	int status = 0;
	std::string header;
	{
        auto req = request(prio::Url("https://www.amazon.de/"));

        fetch(req)
        .then([&status,&header](response res)
        {
			status = res.status();
			header = res.header("server");

			return  timeout(1,0);
		})
		.then( []()
		{
			theLoop().exit();
		})
		.otherwise([](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});

		theLoop().run();

		curl_multi().dispose();
	}

	EXPECT_EQ(200,status);
	EXPECT_EQ("Server",header);
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

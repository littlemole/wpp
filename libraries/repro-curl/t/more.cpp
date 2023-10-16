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


class MoreTest : public ::testing::Test {
 protected:

  virtual void SetUp() 
  {}

  virtual void TearDown() 
  {}
}; // end test setup


TEST_F(MoreTest, SimpleHttp3) 
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	int status = 200;
	{
        auto req1 = request(prio::Url("https://www.amazon.de/"));
        auto req2 = request(prio::Url("https://www.amazon.de/"));

		after(
			fetch(req1),
			fetch(req2)
		)
		.then([&status](std::tuple<response,response> responses)
		{
			response res1;
			response res2;
			std::tie(res1,res2) = responses;

			if(status==200) status = res1.status();
			if(status==200) status = res2.status();

			theLoop().exit();
		})
		.otherwise([](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});

		theLoop().run();

        // can only call that once
		//curl_multi().dispose();
	}

	EXPECT_EQ(200,status);
    // we did not call curl_multi().dispose(), so this won't assert
    //MOL_TEST_ASSERT_CNTS(0, 0);
}


int main(int argc, char **argv) 
{
	prio::Libraries<prio::EventLoop, ReproCurl> init;

	OpenSSL_add_all_algorithms();



    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

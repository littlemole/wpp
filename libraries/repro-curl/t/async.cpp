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


class AsyncTest : public ::testing::Test {
 protected:

  virtual void SetUp() 
  {}

  virtual void TearDown() 
  {}
}; // end test setup




Future<> coroutine_example()
{
	try 
    {
        request req(prio::Url("https://www.google.de/"));

        response res = co_await fetch(req);
std::cout << "1<<<<<<<<<<<<<" << std::endl;
    	int status = res.status();
		std::string header = res.header("server");
std::cout << "2<<<<<<<<<<<<<" << std::endl;

    
    	//EXPECT_EQ(200,status);
	   // EXPECT_EQ("gws",header);
    
		nextTick([](){
			theLoop().exit();
		});
    }
	catch (const std::exception& ex)
	{
        std::cout << "0<<<<<<<<<<<<<" << std::endl;
		std::cout << ex.what() << std::endl;
		theLoop().exit();
	}
    catch (...)
	{
        std::cout << "oioioi" << std::endl;
		theLoop().exit();
	};
    //co_return 42;
}


TEST_F(AsyncTest, asyncTest)
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	{
		coroutine_example().then([](){});

		theLoop().run();
		curl_multi().dispose();
	}

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

#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "reprocpp/after.h"
#include "test.h"
//#include "promise/asio/loop.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include <reprocurl/asyncCurl.h>
#include <curl/curl.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#ifdef _WIN32
#include <openssl/applink.c>
#endif


using namespace prio;
using namespace reprocurl;



repro::Future<> fetch_url(const std::string& url)
{
#ifndef _WIN32
		signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
		signal(SIGINT).then([](int s) { theLoop().exit(); });
	
		auto curl = async_curl()->url(url)->verbose();

		CurlEasy::Ptr result = co_await curl->perform();

		std::cout << result->response_body();

		theLoop().exit();
}


int main(int argc, char **argv) 
{
	prio::Libraries<prio::EventLoop,reprocurl::ReproCurl> init;

	OpenSSL_add_all_algorithms();

	if(argc < 2 )
	{
		std::cout << "usage fetchurl [url]" << std::endl;
		return 0;
	}

	fetch_url(argv[1]);
	
	theLoop().run();

	//MOL_TEST_ASSERT_CNTS(0,0);

    return 0;
}


/*



Future<> coroutine_example();

TEST_F(BasicTest, asyncTest)
{
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int s) { std::cout << "SIGPIPE" << std::endl;});
#endif
		signal(SIGINT).then([](int s) { theLoop().exit(); });

		std::cout << "0:" << std::endl;

		coroutine_example()
		.then([](){})
		.otherwise([](const std::exception& ex){});

		std::cout << "-:" << std::endl;

		theLoop().run();

		std::cout << "+:" << std::endl;

		curl_multi().dispose();
	}


	//EXPECT_EQ(200,status);
	//EXPECT_EQ("gws",header);
	MOL_TEST_ASSERT_CNTS(0, 0);
}



Future<> coroutine_example()
{
	int status = 0;
	std::string header;

	try {

		std::cout << "1:" << std::endl;
		auto req = async_curl()->url("https://www.google.de/");
		std::cout << "2:" << std::endl;

		CurlEasy::Ptr curl = co_await req->perform();
		std::cout << "3:" << std::endl;

		status = curl->status();
		header = curl->response_header("server");

		theLoop().exit();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		theLoop().exit();
	};

}
*/

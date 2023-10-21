#include "priohttp/compress.h"
#include "gtest/gtest.h"
#include <memory>
#include <signal.h>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "reprocpp/after.h"
#include "reprocpp/test.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include "priocpp/connection.h"
#include <event2/thread.h>
#include <event2/event.h>
#include <priohttp/common.h>
#include <priohttp/multipart.h>
#include <priohttp/client.h>
#include <priohttp/http_server.h>
#include <priohttp/request.h>
#include <priohttp/response.h>
#include <priocpp/ssl_connection.h>
#include <cryptoneat/cryptoneat.h>
#include <priohttp/client_conversation.h>
#include <priohttp/server/reader.h>
#include <priohttp/server/writer.h>
#include <priohttp/conversation.h>

using namespace cryptoneat;
using namespace repro;
using namespace prio;


class BasicTest : public ::testing::Test {
 protected:

  static void SetUpTestCase() {


  }

  virtual void SetUp() {
	 // MOL_TEST_PRINT_CNTS();
  }

  virtual void TearDown() {
	 MOL_TEST_PRINT_CNTS();
  }

}; // end test setup

class ConversationSpy : public ReaderWriterConversation
{
public:

	ConversationSpy()
		: req(0),res(0),isError(false)
	{}

	HttpRequest req;
	HttpResponse res;

	std::function<void(std::string)> headersComplete;
	std::function<void(std::string)> requestComplete;
	std::function<void(std::string)> responseComplete;

	std::string reply;
	std::string written;
	
	bool isError;

	virtual void onHeadersComplete(const std::string& s)
	{
		headersComplete(s);
	}

	virtual void onRequestComplete(const std::string& s)
	{
		requestComplete(s);
	}

	virtual void onResponseComplete(const std::string& s)
	{
		responseComplete(s);
	}

	virtual void onRequestError(const std::exception_ptr& )
	{
		isError = true;
	}

	virtual Request& request()
	{
		return req;
	}

	virtual Response& response()
	{
		return res;
	}

	virtual repro::Future<std::string> read()
	{
		auto p = repro::promise<std::string>();

		nextTick()
		.then( [this,p]()
		{
			p.resolve(reply);
		});

		return p.future();
	}

	virtual repro::Future<> write(const std::string& s)
	{
		auto p = repro::promise<>();
		written.append(s);

		nextTick()
		.then( [p]()
		{			
			p.resolve();
		});

		return p.future();
	}

	virtual void flush(Response& ) {}
	virtual void onFlush(std::function<void(Request& req, Response& res)> ) {};
	virtual void chunk(const std::string& ) {};
	virtual Connection::Ptr con() { return nullptr; }

	virtual bool keepAlive()
	{
		return false;
	}
};



TEST_F(BasicTest, HeaderReader)
{
	std::string path;
	std::string body;

	ConversationSpy spy;
	spy.reply = "POST /path HTTP/1.0\r\nContent-Length:10\r\n\r\n1234567890";
	spy.headersComplete = [&body,&path,&spy](std::string s) 
	{
		body = s;
		path = spy.req.path();
		theLoop().exit();		
	};

	HttpHeaderReader hr(&spy);
	hr.consume("");

	signal(SIGINT).then([](int ) { theLoop().exit(); });
	theLoop().run();

	EXPECT_STREQ("/path",path.c_str());
	EXPECT_STREQ("1234567890",body.c_str());
}

TEST_F(BasicTest, BodyReader)
{
	std::string body;

	ConversationSpy spy;
	spy.req.size(10);
	spy.requestComplete = [&body](std::string s)
	{
		body = s;
		theLoop().exit();
	};
	spy.reply = "67890";

	HttpContentLengthBodyReader br(&spy);
	br.consume("12345");

	signal(SIGINT).then([](int ) { theLoop().exit(); });
	theLoop().run();

	EXPECT_STREQ("1234567890",body.c_str());
}

TEST_F(BasicTest, BodyWriter)
{
	ConversationSpy spy;
	spy.res.ok().contentType("text/html").body("0123456789");
	spy.responseComplete = [](std::string )
	{
		theLoop().exit();		
	};

	HttpPlainBodyWriter br(&spy);
	br.flush();

	signal(SIGINT).then([](int ) { theLoop().exit(); });
	theLoop().run();

	EXPECT_STREQ("HTTP/1.1 200 OK\r\ncontent-type:text/html\r\nContent-Length:10\r\n\r\n0123456789",spy.written.c_str());
}

TEST_F(BasicTest, GzipBodyWriter)
{
	ConversationSpy spy;
	spy.res.ok().contentType("text/html").gzip().body("0123456789");
	spy.responseComplete = [](std::string )
	{
		theLoop().exit();		
	};

	auto br = new HttpPlainBodyWriter(&spy);
	HttpGzippedBodyWriter gz(br);
	gz.flush();

	signal(SIGINT).then([](int ) { theLoop().exit(); });
	theLoop().run();

	EXPECT_STREQ("HTTP/1.1 200 OK\r\ncontent-type:text/html\r\nContent-Length:10\r\nCONTENT-ENCODING:gzip\r\n\r\n\x1F\x8B\b",spy.written.c_str());
}

TEST_F(BasicTest, ChunkedBodyWriter)
{
	ConversationSpy spy;
	spy.res.ok().contentType("text/html");
	spy.res.isChunked(true);
	spy.responseComplete = [](std::string )
	{
		theLoop().exit();		
	};

	HttpChunkedBodyWriter br(&spy);
	br.write("a chunk");
	br.write("b chunk");
	br.flush();

	signal(SIGINT).then([](int ) { theLoop().exit(); });
	theLoop().run();

	EXPECT_STREQ("HTTP/1.1 200 OK\r\ncontent-type:text/html\r\nTRANSFER-ENCODING:chunked\r\n\r\n7\r\na chunk\r\n7\r\nb chunk\r\n0\r\n\r\n",spy.written.c_str());
}

TEST_F(BasicTest, GzipChunkedBodyWriter)
{
	ConversationSpy spy;
	spy.res.ok().contentType("text/html").gzip();
	spy.res.isChunked(true);
	spy.responseComplete = [](std::string )
	{
		theLoop().exit();		
	};

	auto br = new HttpChunkedBodyWriter(&spy);
	HttpGzippedBodyWriter gz(br);

	gz.write("a chunk");
	gz.write("b chunk");
	gz.flush();

	signal(SIGINT).then([](int ) { theLoop().exit(); });
	theLoop().run();

	EXPECT_STREQ( "HTTP/1.1 200 OK\r\ncontent-type:text/html\r\nTRANSFER-ENCODING:chunked\r\nCONTENT-ENCODING:gzip\r\n\r\na\r\n\x1F\x8B\b\0\0\0\0\0\0\x3\r\n13\r\nKTH\xCE(\xCD\xCBN\x82P\0&\xE3-\x87\xE\0\0\0\r\n0\r\n\r\n", spy.written.c_str());
}

TEST_F(BasicTest, Compress)
{
	std::string test = "Hello \r\nHello World";

	std::ostringstream oss;

	std::cout << "compress" << std::endl;
	Compressor c;
	oss << c.compress(test);
	oss << c.flush();

	std::string compressed = oss.str();

	EXPECT_STREQ("\x1F\x8B\b\0\0\0\0\0\0\x3\xF3H\xCD\xC9\xC9W\xE0\xE5\xF2\0\xD3\xE1\xF9" "E9)\0f\x86W\xB2\x13\0\0\0",compressed.c_str());

	std::cout << "decompress" << std::endl;

	Decompressor d;
	d.decompress(compressed);
	std::string r = d.flush();

	EXPECT_STREQ(test.c_str(),r.c_str());
}



TEST_F(BasicTest, SimpleHttp) {

	std::string result;

	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ){});
#endif
		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res){
			std::cout << "server start" << std::endl;
			res.ok().flush();
		})
		.otherwise([&httpserver](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
			httpserver.shutdown();
			theLoop().exit();
		});

		TcpConnection::Ptr c;

		TcpConnection::connect("Localhost",8765)
		.then([&c](Connection::Ptr client)
		{
			c = client;
			return client->write("GET /test HTTP/1.0\r\n\r\n");
		})
		.then( [](Connection::Ptr client)
		{
			return client->read();
		})
		.then( [&result,&httpserver](Connection::Ptr client,std::string data)
		{
			std::cout << "client result: " << data << std::endl;
			result = data;

			client->close();
			std::cout << "client closed" << std::endl;
			timeout([&httpserver]()
			{
				std::cout << "timeout" << std::endl;
				httpserver.shutdown();
				theLoop().exit();
			},1,1);
		});

		theLoop().run();
	}
    EXPECT_EQ("HTTP/1.1 200 OK\r\nContent-Length:0\r\n\r\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicTest, SimpleHttpPost100) {

	std::string result;

	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res) {
			std::cout << "server start" << std::endl;
			res.ok().flush();
		})
		.otherwise([&httpserver](const std::exception& ex) {
			std::cerr << "server ex: " << ex.what() << std::endl;
			httpserver.shutdown();
			theLoop().exit();
		});

		TcpConnection::Ptr c;

		TcpConnection::connect("Localhost", 8765)
			.then([&c](Connection::Ptr client)
		{
			c = client;
			return client->write("POST /test HTTP/1.1\r\nContent-length:10\r\nExpect:100-continue\r\n\r\n");
		})
		.then([](Connection::Ptr client)
		{
			return client->read();
		})
		.then([](Connection::Ptr client, std::string data)
		{
			std::cout << "client result: " << data << std::endl;
			return client->write("0123456789");
		})
		.then([](Connection::Ptr client)
		{
			return client->read();
		})
		.then([&result, &httpserver](Connection::Ptr client, std::string data)
		{
			std::cout << "client result: " << data << std::endl;
			result = data;

			client->close();
			std::cout << "client closed" << std::endl;
			timeout([&httpserver]()
			{
				std::cout << "timeout" << std::endl;
				httpserver.shutdown();
				theLoop().exit();
			}, 1, 1);
		});

		theLoop().run();
	}
	EXPECT_EQ("HTTP/1.1 200 OK\r\nContent-Length:0\r\n\r\n", result);
	MOL_TEST_ASSERT_CNTS(0, 0);
}


/* todo fix after
TEST_F(BasicTest, Simple2Http) {

	std::string result;

	{
#ifndef _WIN32
		signal(SIGPIPE)->then([](int s) {});
#endif
		//signal(SIGINT)->then([](int s) { theLoop().exit(); });

		org::http::http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& req, Response& res){
			std::cout << "server start" << std::endl;
			res.ok().flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});

		TcpConnection::Ptr c1;
		TcpConnection::Ptr c2;

		both(
			[&c1](){

				auto p = promise<std::string>();

				TcpConnection::connect("Localhost",8765)
				->then([&c1](org::io::Connection::Ptr client)
				{
					std::cout << "c1 write" << std::endl;
					c1 = client;
					return client->write("GET /test HTTP/1.0\r\n\r\n");
				})
				->then( [](org::io::Connection::Ptr client)
				{
					std::cout << "c1 read" << std::endl;
					return client->read();
				})
				->then( [p](org::io::Connection::Ptr client,std::string data)
				{
					std::cout << "client1 result: " << data << std::endl;
					p->resolve(data);
					//client->close();
					std::cout << "client1 closed" << std::endl;
				})
		->otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});				
				return p->future();
			}(),
			[&c2](){


				auto p = promise<std::string>();

				TcpConnection::connect("Localhost",8765)
				->then([&c2](org::io::Connection::Ptr client)
				{
					std::cout << "c2 write" << std::endl;
					c2 = client;
					return client->write("GET /test HTTP/1.0\r\n\r\n");
				})
				->then( [](org::io::Connection::Ptr client)
				{
					std::cout << "c2 read" << std::endl;
					return client->read();
				})
				->then( [p](org::io::Connection::Ptr client,std::string data)
				{
					std::cout << "client2 result: " << data << std::endl;
					p->resolve(data);
					//client->close();
					std::cout << "client2 closed" << std::endl;
				})
		->otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});				
				return p->future();
			}()
		)
		->then([&result,&httpserver](std::string s1, std::string s2)
		{
			std::cout << "client1+2 closed" << std::endl;
			std::ostringstream oss;
			oss << s1 << "|" << s2;
			result = oss.str();
			httpserver.shutdown();
			theLoop().exit();			
		});

		theLoop().run();
	}

    EXPECT_EQ("HTTP/1.1 200 OK\r\nContent-Length:0\r\n\r\n|HTTP/1.1 200 OK\r\nContent-Length:0\r\n\r\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}
*/

TEST_F(BasicTest, KeepAliveHttp) {

	std::string result;


	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif

		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res){
			std::cout << "server start" << std::endl;
			res.ok().flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});

		TcpConnection::Ptr c;

		timeout( [&result,&c,&httpserver]
		{

			TcpConnection::connect("Localhost",8765)
			.then([&c](Connection::Ptr client)
			{
				c = client;
				return client->write("GET /test HTTP/1.0\r\nConnection:Keep-Alive\r\n\r\n");
			})
			.then( [](Connection::Ptr client)
			{
				return client->read();
			})
			.then( [&result](Connection::Ptr client,std::string data)
			{
				std::cout << "client result: " << data << std::endl;
				result += data;
				return client->write("GET /test HTTP/1.0\r\nConnection:Keep-Alive\r\n\r\n");
			})
			.then( [](Connection::Ptr client)
			{
				return client->read();
			})
			.then( [&result,&httpserver](Connection::Ptr client,std::string data)
			{
				std::cout << "client result: " << data << std::endl;
				result += data;
				client->close();

				timeout( [&httpserver]()
				{
					httpserver.shutdown();
					theLoop().exit();
				},0,1);
			});
		},0,100);

		theLoop().run();
	}

	EXPECT_EQ("HTTP/1.1 200 OK\r\nContent-Length:0\r\nConnection:Keep-Alive\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length:0\r\nConnection:Keep-Alive\r\n\r\n",result);
    MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicTest, SimpleHttpRequest)
{
	std::string result;

	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif

		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res){
			std::cout << "server start" << std::endl;
			res.ok().flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});

		TcpConnection::Ptr c;
		timeout( [&result,&c,&httpserver]
		{

			TcpConnection::connect("Localhost",8765)
			.then([&result,&c,&httpserver](Connection::Ptr client)
			{
				c = client;
				HttpRequest request;

				request.action("GET /test HTTP/1.0");

				HttpClientConversation::on(client,request)
				.then( [&result,&httpserver](Request& /*req*/, Response& res)
				{
					std::cout << res.status() << std::endl;
					result = res.status();
					httpserver.shutdown();
					theLoop().exit();

				});
			});

		},0,100);

		theLoop().run();
	}
	EXPECT_EQ("HTTP/1.1 200 OK",result);
	MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicTest, SimpleHttpRequest2)
{
	std::string result;
	int status = 0;

	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ){theLoop().exit();});

		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res){
			std::cout << "server start" << std::endl;
			res.ok().flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});

		timeout( [&result,&status,&httpserver]()
		{
			HttpClient::url("http://localhost:8765/test")
			->GET()
			.then( [&result,&status,&httpserver](Response& res)
			{
				std::cout << res.status() << std::endl;
				result = res.status();
				status = res.statusCode();
				httpserver.shutdown();
				theLoop().exit();			
			})
			.otherwise( [](const std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
			});
		},0,100);

		theLoop().run();
	}

	MOL_TEST_ASSERT_CNTS(0,0);
	EXPECT_EQ("HTTP/1.1 200 OK",result);
	EXPECT_EQ(200,status);
}


TEST_F(BasicTest, ChunkedHttpResponse)
{
	std::string result;
	int status = 0;

	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ){theLoop().exit();});

		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res){

			res.ok();
			res.chunk("a chunk");
			res.chunk("b chunk");
			res.flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});

		timeout( [&result,&status,&httpserver]()
		{
			HttpClient::url("http://localhost:8765/test")
			->GET()
			.then( [&result,&status,&httpserver](Response& res)
			{
				std::cout << res.status() << std::endl;
				result = res.body();
				status = res.statusCode();

				timeout( [&httpserver]()
				{
					httpserver.shutdown();
					theLoop().exit();
				},0,1);
			})
			.otherwise( [](const std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
			});
		},0,100);

		theLoop().run();
	}

	EXPECT_EQ("a chunkb chunk",result);
	EXPECT_EQ(200,status);
	MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicTest, ChunkedHttpResponse2)
{
	std::string result;
	int status = 0;

	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ){theLoop().exit();});

		http_server httpserver;
		httpserver.bind(8765)//.listen(10)
		.then([](Request& /*req*/, Response& res){

			res.ok();
			res.chunk("a chunk");
			return timeout( [&res]()
			{
				res.chunk("bb chunk");
				res.flush();
			},0,100);

		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});

		timeout( [&result,&status,&httpserver]()
		{
			HttpClient::url("http://localhost:8765/test")
			->GET()
			.then( [&result,&status,&httpserver]( Response& res)
			{
				std::cout << res.status() << std::endl;
				result = res.body();
				status = res.statusCode();

				timeout( [&httpserver]()
				{
					httpserver.shutdown();
					theLoop().exit();
				},0,1);
			})
			.otherwise( [](const std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
			});
		},0,100);

		theLoop().run();
	}

	EXPECT_EQ("a chunkbb chunk",result);
	EXPECT_EQ(200,status);
	MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicTest, SimpleSSLRequest)
{
	std::string result;

	{
		SslCtx server_ctx;
		server_ctx.load_cert_pem("pem/server.pem");

		SslCtx client_ctx;
		client_ctx.set_ca_path("pem/ca.crt");
		
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ){theLoop().exit();});

		http_server httpserver(server_ctx);
		httpserver.bind(8765)//.listen(10,server_ctx)
		.then([](Request& /*req*/, Response& res){
			std::cout << "server start" << std::endl;
			res.body("HELO WORLD");
			res.ok().flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});


		SslConnection::Ptr c;
		timeout( [&c,&result,&client_ctx,&httpserver]
		{
			SslConnection::connect("Localhost",8765,client_ctx)
			.then([&result,&c,&httpserver](Connection::Ptr client)
			{
				c = client;
				HttpRequest request;

				request.action("GET /test HTTP/1.0");

				HttpClientConversation::on(client,request)
				.then( [&result,&httpserver](Request& /*req*/, Response& res)
				{
					std::cout << res.status() << std::endl;
					result = res.status();

					timeout( [&httpserver]()
					{
						httpserver.shutdown();
						theLoop().exit();
					},0,1);
				});
			});

		},0,100);

		theLoop().run();
	}

	MOL_TEST_ASSERT_CNTS(0,0);
	EXPECT_EQ("HTTP/1.1 200 OK",result);
}



TEST_F(BasicTest, HeaderTest1)
{
	std::string header = "form-data; name=\"fieldName\"; filename=\"filename.jpg\"";

	Headers headers;
	headers.set("Content-Disposition", header);

	auto h = headers.values("Content-Disposition");
	auto value = h.value().main();

	EXPECT_EQ("form-data",value);

	auto p1 = h.value().params()["name"];

	EXPECT_EQ("fieldName",p1);

	auto p2 = h.value().params()["filename"];

	EXPECT_EQ("filename.jpg",p2);
}


TEST_F(BasicTest, HeaderTest2)
{
	std::string header = "text/html; charset=ISO-8859-4";

	Headers headers;
	headers.set("Content-Type", header);

	auto h = headers.values("Content-Type");
	auto value = h.value().main();

	EXPECT_EQ("text/html",value);

	auto p1 = h.value().params()["charset"];

	EXPECT_EQ("ISO-8859-4",p1);

}


TEST_F(BasicTest, HeaderTest3)
{
	std::string header = "text/plain; q=0.5, text/html,text/x-dvi; q=0.8, text/x-c";

	Headers headers;
	headers.set("Accept", header);

	auto h = headers.values("Accept");
	auto value = h.value().main();

	EXPECT_EQ("text/plain",value);

	EXPECT_EQ(4,h.size());

	EXPECT_EQ("text/plain",h[0].main());
	EXPECT_EQ("0.5",h[0].params()["q"]);

	EXPECT_EQ("text/html",h[1].main());

	EXPECT_EQ("text/x-dvi",h[2].main());
	EXPECT_EQ("0.8",h[2].params()["q"]);

	EXPECT_EQ("text/x-c",h[3].main());
}


TEST_F(BasicTest, HeaderTest4)
{
	std::string header = "de, en, da";

	Headers headers;
	headers.set("Accept-Language", header);

	auto h = headers.values("Accept-Language");
	auto value = h.value().main();

	EXPECT_EQ("de",value);

	EXPECT_EQ(3,h.size());

	EXPECT_EQ("de",h[0].main());
	EXPECT_EQ("en",h[1].main());
	EXPECT_EQ("da",h[2].main());
}

TEST_F(BasicTest, args)
{
	bool result = false;

	HttpRequest req;
	req.parse("GET /app/app_mgr HTTP/1.0\r\n\r\n");

	std::regex rgx("/app/([^\\/]*)");
	std::vector<std::string> args;
	args.push_back("client_id");

	if (req.match("GET", rgx, args))
	{
		std::cout << req.path.path() << std::endl;
		std::cout << req.path.args().get("client_id") << std::endl;

		if ( req.path.args().get("client_id") == "app_mgr" )
		{
			result = true;
		}
	}

	EXPECT_EQ(true,result);
}


// 						   ---------------------------9051914041544843365972754266

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


TEST_F(BasicTest, MultiPartParser)
{
	MultiParts mp(multipart,"---------------------------9051914041544843365972754266");

	std::cout << mp.parts[0].body << std::endl;

	EXPECT_STREQ("text default",mp.parts[0].body.c_str() );
	EXPECT_STREQ("Content of a.txt.\r\n",mp.parts[1].body.c_str() );
	EXPECT_STREQ("<!DOCTYPE html><title>Content of a.html.</title>\r\n",mp.parts[2].body.c_str() );

}

TEST_F(BasicTest, SimpleHttp2SSLRequest)
{
	int result = 500;

	{
		Http2SslCtx server_ctx;
		server_ctx.load_cert_pem("pem/server.pem");
		server_ctx.enableHttp2();

		Http2SslCtx client_ctx;
		client_ctx.set_ca_path("pem/ca.crt");
		client_ctx.enableHttp2Client();
		
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ){theLoop().exit();});

		http_server httpserver(server_ctx);
		httpserver.bind(8765)//.listen(10,server_ctx)
		.then([](Request& /*req*/, Response& res){
			std::cout << "server start" << std::endl;
			res.body("HELO WORLD");
			res.ok().flush();
		})
		.otherwise([](const std::exception& ex){
			std::cerr << "server ex: " << ex.what() << std::endl;
		});


		SslConnection::Ptr c;
		timeout( [&c,&result,&client_ctx,&httpserver]
		{
			SslConnection::connect("Localhost",8765,client_ctx)
			.then([&result,&c,&httpserver](Connection::Ptr client)
			{
				c = client;
				HttpRequest request;

				request.action("GET /test HTTP/1.0");
				request.header("HOST","localhost:8765");

				Http2ClientConversation::on(client,request)
				.then( [&result,&httpserver](Request& /*req*/, Response& res)
				{
					std::cout << res.status() << std::endl;
					result = res.statusCode();
					//c->close();
					
					timeout( [&httpserver]()
					{
						httpserver.shutdown();
						theLoop().exit();
					},0,1);
				});
			});

		},0,100);

		theLoop().run();
	}
	// NOTE: http2 uses keep alive by default so we will not tear down to zero here
	//MOL_TEST_ASSERT_CNTS(0,0);
	EXPECT_EQ(200,result);
}



TEST_F(BasicTest, PathTest1)
{
	std::string self = prio::get_current_work_dir();

	std::string test = prio::real_path(".");

	EXPECT_EQ(self,test);

	std::string test2 = prio::real_path("/.");

#ifdef _WIN32
	EXPECT_STRCASEEQ("C:\\",test2.c_str());
#else
	EXPECT_EQ("/",test2);
#endif
}

int main(int argc, char **argv) 
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

	::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

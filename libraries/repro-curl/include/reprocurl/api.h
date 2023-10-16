#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CLIENT_CURL_API_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CLIENT_CURL_API_DEF_GUARD_

#include "reprocurl/asyncCurl.h"
#include "priocpp/url.h"
  
namespace reprocurl {


class response;
class request;

repro::Future<response> fetch(request& req);

class request
{
	friend repro::Future<response> fetch(request& req);
public:

	request();
	request(const prio::Url& url);

	request& url(const prio::Url& url);
	request& method(const std::string& method);
	request& data(const std::string& payload);
	request& header(const std::string& header, const std::string& value);
	request& content_type(const std::string& value);
	request& accept(const std::string& value);
	request& user_agent(const std::string& value);
	request& insecure();
	request& verbose();
	request& ca_file(const std::string& value);
	request& client_cert(const std::string& value);


private:
	std::string ca_file_;
	std::string client_cert_;
	std::string method_;
	prio::Url url_; 
	std::string payload_;
	std::vector<std::pair<std::string, std::string>> headers_;
	bool insecure_ = false;
	bool verbose_ = false;
};

class response
{
public:

	response();
	response(CurlEasy::Ptr c);

	long status();
	std::string content();
	std::string content_type();
	std::string header(const std::string& key);

	std::vector<std::pair<std::string, std::string>> headers();

private:
	CurlEasy::Ptr curl_;
};


repro::Future<std::vector<response>> fetch_all(const std::vector<request>& requests);

}

#endif

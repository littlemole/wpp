#include "gtest/gtest.h"

#include "priocpp/loop.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include "priohttp/http_server.h"
#include "priohttp/request.h"
#include "priohttp/response.h"
#include "priohttp/conversation.h"
#include "priohttp/client_conversation.h"
#include "priohttp/client.h"

#include <reproweb/ctrl/controller.h>
#include <reproweb/tools/config.h>
#include <reproweb/json/jwt.h>
#include <reproweb/ctrl/ssi.h>
#include <reprocurl/asyncCurl.h>
#include <reproweb/ctrl/front_controller.h>

#include <reproweb/json/json.h>
#include <reproweb/serialization/json.h>
#include <reproweb/serialization/xml.h>
#include <reproweb/serialization/web.h>

#include <reproweb/web_webserver.h>
#include <reproweb/view/i18n.h>
#include <reproweb/view/tpl.h>
#include <reproweb/tools/validation.h>
#include <cryptoneat/cryptoneat.h>
#include <cryptoneat/base64.h>
#include <diycpp/ctx.h>

 
using namespace reproweb;
using namespace repro;
using namespace prio;
using namespace cryptoneat;
using namespace diy;

typedef const char* entity_name_t;

 
struct Input 
{
	std::string id;
	std::string filter;
	std::string sid;
	prio::HeaderValues header;
	prio::Cookie cookie;
	std::string lang;


};
 
template<>
struct meta::Data<Input>
{
        static constexpr auto meta()
        {
                return meta::data(
                        entity_root("Input"),
						// note some mappings are duplicated
						// for serialization, last one will overider earlier ones
						// for deserialization however, all will be mapped
						"id", &Input::id,
						"filter", &Input::filter,
						"sid", &Input::sid,
						"Accept-Language", &Input::header,
						"Accept-Language", &Input::lang,
						"sid", &Input::cookie
                );
        }
};


struct User
{
public:

	std::string username;
	std::string login;
	std::string pwd;
	std::vector<std::string> tags;
};

template<>
struct meta::Data<User>
{
        static constexpr auto meta()
        {
                return meta::data(
					entity_root("user"),
					"username", &User::username,
					"login", &User::login,
					"pwd", &User::pwd,
					"tags", &User::tags
				);
		}
};
 

void validate(User& user)
{
	std::cout << "validate user: " << JSON::stringify(meta::toJson(user)) << std::endl;
	reproweb::valid( user.username, std::regex("[^<>&]+"), "invalid username");
	reproweb::valid( user.login, std::regex("[^<>&]+"), "invalid login");
	reproweb::valid( user.pwd, std::regex(".+"), "invalid pwd");
	reproweb::valid( user.tags, std::regex("[0-9a-zA-Z]+"), "invalid tags");
}
 

class Logger
{
public:

	void log(const std::string& s)
	{
		std::cout << s << std::endl;
	}
 

	repro::Future<int> test()
	{
		(void)(co_await nextTick());
		co_return 42;
	}

};


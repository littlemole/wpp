#include "webtest.h"

#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif


class BasicTest : public ::testing::Test {
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



TEST_F(BasicTest, SimpleConfig)
{
	Config config;
	config.load("./config.json");

	EXPECT_STREQ("/test", config.getString("url").c_str() );
}


TEST_F(BasicTest, EventBus)
{
	std::string result;

	EventBus eventBus;

	eventBus.subscribe( "test", [&result](Json::Value value)
	{
		result = value["event"].asString();
		theLoop().exit();
	});

	timeout( [&eventBus]()
	{
		Json::Value value(Json::objectValue);
		value["event"] = "TEST";
		eventBus.notify("test", value);
	},0,100);

	theLoop().run();

	EXPECT_EQ("TEST",result);
	MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicTest, EventBusMore)
{
	int result = 0;

	EventBus eventBus;

	eventBus.subscribe( "test", [&result](Json::Value value)
	{
		result++;
	});

	eventBus.subscribe( "test", [&result](Json::Value value)
	{
		result++;
	});

	eventBus.subscribe( "test2", [&result](Json::Value value)
	{
		result = -1;
	});

	timeout( [&eventBus]()
	{
		Json::Value value(Json::objectValue);
		value["event"] = "TEST";
		eventBus.notify("test", value);
		timeout([]()
		{
			theLoop().exit();
		},0,100);
	},0,100);


	theLoop().run();

	EXPECT_EQ(2,result);
	MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicTest, EventBusMoreButLess)
{
	int result = 0;

	EventBus eventBus;

	std::string id = eventBus.subscribe( "test", [&result](Json::Value value)
	{
		result++;
	});

	eventBus.subscribe( "test", [&result](Json::Value value)
	{
		result++;
	});

	eventBus.subscribe( "test2", [&result](Json::Value value)
	{
		result = -1;
	});

	eventBus.unsubscribe("test",id);

	timeout( [&eventBus]()
	{
		Json::Value value(Json::objectValue);
		value["event"] = "TEST";
		eventBus.notify("test", value);
		timeout([]()
		{
			theLoop().exit();
		},0,100);
	},0,100);

	theLoop().run();

	EXPECT_EQ(1,result);
	MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicTest, JWT)
{
	std::string secret = "verysecretsecret";

	Json::Value claim(Json::objectValue);
	claim["data"] = "some payload";
	reproweb::JWT jwt(claim);

	std::string token = jwt.sign(secret,60);

	std::cout << "token: " << token << "<-----" << std::endl;

	reproweb::JWT verifier(token);
	bool verified = verifier.verify(secret);
 
	EXPECT_EQ(true,verified);  

	EXPECT_STREQ("some payload", verifier.claim()["data"].asString().c_str());
}


TEST_F(BasicTest, JWTRSASHA256)
{
	cryptoneat::PublicKey pubkey("pem/public.pem");
	cryptoneat::PrivateKey privkey("pem/private.pem");

    std::time_t exp = time(NULL);
    exp += 60;
    Json::Value claim_(Json::objectValue);
    Json::Value header_(Json::objectValue);

    header_["typ"] = "JWT";
    header_["alg"] = "RS256";
    claim_["exp"] = exp;
	claim_["data"] = "some payload";

    std::cout << "exp:" << exp << std::endl;

    std::ostringstream oss;
    oss << Base64Url::encode(JSON::stringify(header_));
    oss << ".";
    oss << Base64Url::encode(JSON::stringify(claim_));
    std::string payload = oss.str();

    cryptoneat::Signature signer(digest("sha256"),privkey);
    std::string signature_ = signer.sign(payload);


    std::string b64 = Base64Url::encode(signature_);
    std::string tmp = Base64Url::decode(b64);

    std::ostringstream token;
    token << payload << "." << Base64Url::encode(signature_);

	std::cout << token.str() << std::endl;

    cryptoneat::Signature verifier(digest("sha256"),pubkey);
    bool b = verifier.verify(payload,signature_);

    EXPECT_EQ(true,b);


    b = verifier.verify(payload,tmp);

    EXPECT_EQ(true,b);
}

TEST_F(BasicTest, JWTRSASHA25__6)
{
	cryptoneat::PublicKey pubkey("pem/public.pem");
	cryptoneat::PrivateKey privkey("pem/private.pem");

	Json::Value claim(Json::objectValue);
	claim["data"] = "some payload";

	reproweb::JWT jwt(claim);
	std::string token = jwt.sign(privkey,60);

	reproweb::JWT verifier(token);
	bool verified = verifier.verify(pubkey);

	EXPECT_EQ(true,verified);

	EXPECT_STREQ("some payload", verifier.claim()["data"].asString().c_str());

}

TEST_F(BasicTest,dummy)
{
	std::string in = "xlPa8hRIGeEhwSJjZ-D0a1U9-f_GvNWm_OiFyKrmzERmYqiuilCtDewMVnQsvC2gUzaJpz6geJzzqJMeRFsuxhkU8YC75Bd_B-JF0FaY5I2S9ogxarBEdnSA17TYeKbao1Kehq47GfPt6xF5I0HcZlsJ7MeuzDJh_NUIRCCEMTPYCRYDNknyUmznwWfgZyD5t6JcpL7WdxDAAVfxXWZXBOEmDQcV0qCh0cnct3xgixa83c-YjjfO6SBUdruzPXefyrkBqG08szx-5VSvIA1QB3NvqodkfL0PIYDYVwwrjRue9E1dbZ57Tt2P6zw8YZ8zA9MWutu8WhB7gfgdtim_";

    std::string plain = Base64Url::decode(in);

    std::string b64 = Base64Url::encode(plain);

    plain = Base64Url::decode(in);
 
    EXPECT_STREQ(b64.c_str(),in.c_str());
}


TEST_F(BasicTest,quote)
{
	std::string in = "some\"text\"with\"embedded\"quotes";

    std::string escaped = reproweb::csv_quote(in);

    EXPECT_STREQ("\"some\"\"text\"\"with\"\"embedded\"\"quotes\"",escaped.c_str());
}

 
TEST_F(BasicTest, Invocable) 
{
	User user{ "mike", "littlemole", "secret", { "one", "two", "three"} };

	std::cout << "invocable: " << reproweb::has_valid<User>::value << std::endl;

	if ( reproweb::has_valid<User>::value )
	{
		reproweb::call_valid::invoke(user);
	}

	Logger logger;

	std::cout << "invocable: " << reproweb::has_valid<Logger>::value << std::endl;

	if ( reproweb::has_valid<Logger>::value )
	{
		reproweb::call_valid::invoke(logger);
	}

}


 
TEST_F(BasicTest, NamedArgs) 
{
	User user{ "mike", "littlemole", "secret" };

	std::string sql = "SELECT username,login,pwd FROM User WHERE login = :login AND pwd = :pwd";

	std::ostringstream oss;
	bool inside_quote = false;

	std::vector<std::string> named_args;

	char last = 0;
	size_t pos = 0;
	while( pos < sql.size() )
	{
		char c = sql[pos];
		if ( c == '\'' && last != '\\')
		{
			if(inside_quote) 
				inside_quote = false;
			else
				inside_quote = true;
		} 

		if(!inside_quote)
		{
			if ( c == ':' )
			{
				size_t p = sql.find_first_of(" \t\r\n",pos);
				if( p != std::string::npos)
				{
					std::string named_arg = sql.substr(pos+1,p-pos-1);
					named_args.push_back(named_arg);
					oss << "? ";
					pos += p-pos;
					last = ' ';
					continue;
				} 
				else
				{
					std::string named_arg = sql.substr(pos+1);
					named_args.push_back(named_arg);
					oss << "? ";
					pos += named_arg.size()+1;
					last = ' ';
					continue;
				}
			}
			else
			{
				oss << c;
			}
		}
		else
		{
			oss << c;
		}
		pos++;
		last = c;
	} 

	std::cout << oss.str() << std::endl;

	for ( auto& na : named_args)
	{
		std::cout << na << std::endl;
	}
}

 
TEST_F(BasicTest, MetaFind) 
{
	User user{ "mike", "littlemole", "secret", { "one", "two", "three"} };

//	auto m = meta_of(user);

/*
	std::string username ;
	m.find<std::string>("username",[&user,&username](auto& m)
	{
		username = m.get(user);
	});
*/
	std::string username = meta::get<std::string>(user,"username");

	EXPECT_EQ("mike",username);

	meta::set(user,"username",std::string("mole"));

	EXPECT_EQ("mole",user.username);

	bool r = meta::has_getter<std::string>(user,"username");

	EXPECT_EQ(true,r);

	r = meta::has_getter<std::string>(user,"username2");

	EXPECT_EQ(false,r);

}


int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

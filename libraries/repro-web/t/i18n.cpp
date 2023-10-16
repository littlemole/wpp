#include "webtest.h"

#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif


class BasicI18NTest : public ::testing::Test {
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



TEST_F(BasicI18NTest, SimpleI18N) 
{
	I18N i18n("/locale/properties", {"de", "de_DE", "en"});

	std::string val = i18n.key("de","login.page.title");
	EXPECT_EQ("Hallo Coro Websockets",val);

	val = i18n.key("de_DE","login.page.title");
	EXPECT_EQ("Guten Tag Coro Websockets",val);

	val = i18n.key("en_US","login.page.title");
	EXPECT_EQ("Heya Coro Websockets",val);	

	val = i18n.key("en","login.page.title");
	EXPECT_EQ("Heya Coro Websockets",val);	

	val = i18n.key("","login.page.title");
	EXPECT_EQ("Hello Coro Websockets",val);	

	val = i18n.key("xxxunknownlocalexxx","login.page.title");
	EXPECT_EQ("Hello Coro Websockets",val);	

}


 
TEST_F(BasicI18NTest, SimpleI18Ndefaults) 
{
	I18N i18n("/locale/properties", {"de", "de_DE", "en"});

	std::string val = i18n.key("de","login.main.copyright");
	EXPECT_EQ("&copy; littlemole. All rights reserved.",val);

	val = i18n.key("de_DE","login.main.copyright");
	EXPECT_EQ("&copy; littlemole. All rights reserved.",val);

	val = i18n.key("de_DE","login.main.copyright");
	EXPECT_EQ("&copy; littlemole. All rights reserved.",val);

	val = i18n.key("de_DE","login.main.copyright");
	EXPECT_EQ("&copy; littlemole. All rights reserved.",val);
}


TEST_F(BasicI18NTest, SimpleI18Ntpl) 
{
	I18N i18n("/locale/properties", {"de", "de_DE", "en"});

	std::string tpl = 
	"<html>\n"
	"<body>\n"
	"<h1><!--#i18n key='login.page.title' --></h1>\n"
	"<p><!--#i18n key = 'login.header.headline.desc' --></p>\n"
	"</body>\n"
	"</html>\n";

	std::string content = i18n.render("de_DE",tpl);
	EXPECT_EQ("<html>\n<body>\n<h1>Guten Tag Coro Websockets</h1>\n<p>modernes reactives c++ f\xC3\xBCr das web.</p>\n</body>\n</html>\n",content);

	content = i18n.render("de",tpl);
	EXPECT_EQ("<html>\n<body>\n<h1>Hallo Coro Websockets</h1>\n<p>modernes reactives c++ f\xC3\xBCr das web.</p>\n</body>\n</html>\n",content);

	content = i18n.render("en",tpl);
	EXPECT_EQ("<html>\n<body>\n<h1>Heya Coro Websockets</h1>\n<p>modern reactive c++ for the web.</p>\n</body>\n</html>\n",content);

	content = i18n.render("en_US",tpl);
	EXPECT_EQ("<html>\n<body>\n<h1>Heya Coro Websockets</h1>\n<p>modern reactive c++ for the web.</p>\n</body>\n</html>\n",content);

	content = i18n.render("en_UK",tpl);
	EXPECT_EQ("<html>\n<body>\n<h1>Heya Coro Websockets</h1>\n<p>modern reactive c++ for the web.</p>\n</body>\n</html>\n",content);

	content = i18n.render("xyz",tpl);
	EXPECT_EQ("<html>\n<body>\n<h1>Hello Coro Websockets</h1>\n<p>modern reactive c++ for the web.</p>\n</body>\n</html>\n",content);

}

TEST_F(BasicI18NTest, I18NtplWithMarkup) 
{
	I18N i18n("/locale/properties", {"de", "de_DE", "en"});

	std::string tpl = 
	"<html>\n"
	"<body>\n"
	"<h1><!--#i18n key='login.page.title' --></h1>\n"
	"email: {{ login }}\n"
	"<p><!--#i18n key = 'login.main.greeting' --></p>\n"
	"</body>\n"
	"</html>\n";     
 
	Json::Value json(Json::objectValue);
	json["username"] = "Michael";
	json["login"]    = "littlemole@oha7.org";

	std::string markup = i18n.render("de_DE",tpl);
	std::string content = mustache::render(markup,json);

	EXPECT_EQ("<html>\n<body>\n<h1>Guten Tag Coro Websockets</h1>\nemail: littlemole@oha7.org\n<p>Willkomen zur\xC3\xBC" "ck, Michael!</p>\n</body>\n</html>\n",content);

	content = mustache::render(i18n.render("de",tpl),json);
	EXPECT_EQ("<html>\n<body>\n<h1>Hallo Coro Websockets</h1>\nemail: littlemole@oha7.org\n<p>Willkomen zur\xC3\xBC" "ck, Michael!</p>\n</body>\n</html>\n",content);

	content = mustache::render(i18n.render("en",tpl),json);
	EXPECT_EQ("<html>\n<body>\n<h1>Heya Coro Websockets</h1>\nemail: littlemole@oha7.org\n<p>Hello dear Michael</p>\n</body>\n</html>\n",content);

	content = mustache::render(i18n.render("en_US",tpl),json);
	EXPECT_EQ("<html>\n<body>\n<h1>Heya Coro Websockets</h1>\nemail: littlemole@oha7.org\n<p>Hello dear Michael</p>\n</body>\n</html>\n",content);

	content = mustache::render(i18n.render("en_UK",tpl),json);
	EXPECT_EQ("<html>\n<body>\n<h1>Heya Coro Websockets</h1>\nemail: littlemole@oha7.org\n<p>Hello dear Michael</p>\n</body>\n</html>\n",content);

	content = mustache::render(i18n.render("xyz",tpl),json);
	EXPECT_EQ("<html>\n<body>\n<h1>Hello Coro Websockets</h1>\nemail: littlemole@oha7.org\n<p>Hello dear Michael</p>\n</body>\n</html>\n",content);

}
 
 
 

int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

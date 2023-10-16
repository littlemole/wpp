#include "webtest.h"

#include "reprocpp/test.h"

#ifdef _WIN32
#include <openssl/applink.c>
#endif 

using namespace patex;
using namespace meta;

class BasicSerializationTest : public ::testing::Test {
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




class Users 
{
public:
	std::string something;
	std::vector<User> users;

	static constexpr auto meta()
	{
		return meta::data(
			"something", &Users::something,
			"users", &Users::users
		);
	}
};

class ArrayTest 
{
public:

	std::vector<std::vector<int>> array;

	static constexpr auto meta()
	{
		return meta::data(
			"array", &ArrayTest::array
		);
	}
};

class Dummy
{
public:
	int x = 0;

	static constexpr auto meta()
	{
		return meta::data(
			"value", &Dummy::x
		);
	}	
};
 
TEST_F(BasicSerializationTest, fromParamsInt) 
{
	QueryParams qp("value=8");

	Dummy d;
	fromParams(qp,d);
 
	EXPECT_EQ(8,d.x);
}


TEST_F(BasicSerializationTest, fromParams) 
{
	QueryParams qp("username=mike,thumes&login=littlemole&pwd=secret&tags=one,two,three");

	User user;
	fromParams(qp,user);

	EXPECT_EQ("mike,thumes",user.username);
	EXPECT_EQ("littlemole",user.login);
	EXPECT_EQ("secret",user.pwd);

	EXPECT_EQ("one",user.tags[0]);
	EXPECT_EQ("two",user.tags[1]);
	EXPECT_EQ("three",user.tags[2]);
}
   
 
template<class T>
std::string joinTabs(const std::vector<T>& v)
{
	if(v.empty())
	return "";

	std::ostringstream oss;

	for( auto i : v)
	{
		oss << csv_quote(i);
		oss << '\t';
	}

	std::string result = oss.str();
	return result.substr(0,result.size()-1);
}

/*
template<class T>
std::string toCSV(const std::vector<T>& v)
{
	std::ostringstream oss;
	auto m = meta_of<T>();

	oss << joinTabs(meta_fields_of<T>()) << std::endl;
	for( auto i : v)
	{
		std::vector<std::string> result;
		auto visitor = [&i,&result]( const char* n, auto& m )
		{	
			std::ostringstream oss;
			oss << m.get(i);
			result.push_back(oss.str());
		};

		m.visit(i,visitor); 

		oss << joinTabs(result) << std::endl;
	}

	return oss.str();
}

class CSVTest
{
public:

	std::string columna;
	int columnb;
	long columnc;

	auto meta() const
	{
		return metadata<CSVTest>(
			"columna", &CSVTest::columna,
			"columnb", &CSVTest::columnb,
			"columnc", &CSVTest::columnc
		);
	}
};
 
TEST_F(BasicSerializationTest, toCSV) 
{
	CSVTest t{ "a value", 42, 4711};

	std::vector<CSVTest> root;
	root.push_back(t);
	root.push_back(t);
	root.push_back(t);  
   
	std::string s = toCSV(root);

	EXPECT_EQ("\"columna\"\t\"columnb\"\t\"columnc\"\n\"a value\"\t\"42\"\t\"4711\"\n\"a value\"\t\"42\"\t\"4711\"\n\"a value\"\t\"42\"\t\"4711\"\n",s);
}

    */


int main(int argc, char **argv)
{
	prio::Libraries<prio::EventLoop,cryptoneat::SSLUser> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

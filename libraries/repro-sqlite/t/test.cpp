#define _XOPEN_SOURCE       /* See feature_test_macros(7) */

#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "reprocpp/after.h"
#include "reprocpp/test.h"
#include <json/json.h>
#include "priocpp/loop.h"
#include "priocpp/api.h"
#include "priocpp/task.h"

#include <event2/thread.h>
#include <event2/event.h>

#include <reprosqlite/sqlite.h>
#include <reprosqlite/sqlite-json.h>

#include <stdio.h>
#include <sqlite3.h>
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <crypt.h>
#endif

using namespace prio;

//namespace {

static int callback(void* /*NotUsed*/, int argc, char **argv, char **azColName)
{
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}


class BasicTest : public ::testing::Test {
 protected:

  static void SetUpTestCase() {


  }

  virtual void SetUp() {
	 // MOL_TEST_PRINT_CNTS();
  }

  virtual void TearDown() {
	 // MOL_TEST_PRINT_CNTS();
  }



}; // end test setup




TEST_F(BasicTest, SimpleSqlite) {

    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("test.db", &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return;
    }
    rc = sqlite3_exec(db, "SELECT * FROM user;", callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);

}

TEST_F(BasicTest, SimpleSql) {

	std::string result;
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ){});
#endif
		signal(SIGINT).then([](int ) { theLoop().exit(); });

		reprosqlite::SqlitePool sql("test.db");

		sql.stm("SELECT * FROM user")->exec()
		.then([&result](reprosqlite::Result r)
		{
			std::ostringstream oss;
			for( unsigned int i = 0; i < r.rows(); i++)
			{
				for( unsigned int j = 0; j < r.cols(); j++)
				{
					oss << r.row(i)[j];
				}
				oss << std::endl;
			}
			result = oss.str();
			theLoop().exit();
		})
		.otherwise([](const std::exception& )
		{

		});
		theLoop().run();
	}

	EXPECT_EQ("1admin12345\n2mike12345\n",result);
	MOL_TEST_ASSERT_CNTS(0,0);
}


TEST_F(BasicTest, SimpleSqlJson) {

	std::string result;
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ){});
#endif
		signal(SIGINT).then([](int ) { theLoop().exit(); });

		reprosqlite::SqlitePool sql("test.db");

		sql.stm("SELECT * FROM user")->exec()
		.then([&result](reprosqlite::Result r)
		{
			Json::StreamWriterBuilder wbuilder;
			wbuilder["commentStyle"] = "None";
			wbuilder["indentation"] = ""; 

			result = Json::writeString(wbuilder, toJson(r));
			theLoop().exit();
		})
		.otherwise([](const std::exception& )
		{

		});
		theLoop().run();
	}

	EXPECT_EQ("[{\"email\":\"admin\",\"id\":\"1\",\"passwd\":\"12345\"},{\"email\":\"mike\",\"id\":\"2\",\"passwd\":\"12345\"}]",result);
	MOL_TEST_ASSERT_CNTS(0,0);
}

TEST_F(BasicTest, SimpleSql2) {

	std::string result;
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ) { theLoop().exit(); });

		reprosqlite::SqlitePool sql("test.db");

		sql.query("SELECT * FROM user WHERE id = ?", 1)
		.then([&result](reprosqlite::Result r)
		{
			std::ostringstream oss;
			for (unsigned int i = 0; i < r.rows(); i++)
			{
				for (unsigned int j = 0; j < r.cols(); j++)
				{
					oss << r.row(i)[j];
				}
				oss << std::endl;
			}
			result = oss.str();
			theLoop().exit();
		})
			.otherwise([](const std::exception& )
		{

		});
		theLoop().run();
	}

	EXPECT_EQ("1admin12345\n", result);
	MOL_TEST_ASSERT_CNTS(0, 0);
}

TEST_F(BasicTest, SimpleSqlFailconstraint) {

	std::string result;
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int ) {});
#endif
		signal(SIGINT).then([](int ) { theLoop().exit(); });

		reprosqlite::SqlitePool sql("test.db");

		sql.query("INSERT INTO user (id,email,passwd) VALUES (?,?,?)", 1,"wont work", "nada")
		.then([&result](reprosqlite::Result r)
		{
			std::cout << "worked" << std::endl;
			std::ostringstream oss;
			for (unsigned int i = 0; i < r.rows(); i++)
			{
				for (unsigned int j = 0; j < r.cols(); j++)
				{
					oss << r.row(i)[j];
				}
				oss << std::endl;
			}
			result = oss.str();
			theLoop().exit();
		})
		.otherwise([&result](const std::exception& ex)
		{
			std::cout << "failed:" << typeid(ex).name() << "|"<< ex.what() << std::endl;
			result = "failed";
			theLoop().exit();
		});
		theLoop().run();
	}

	EXPECT_EQ("failed", result);
	MOL_TEST_ASSERT_CNTS(0, 0);
}

/*
class SaltedPasswd
{
public:

	SaltedPasswd()
	{
	}

	std::string encrypt(const std::string& clear)
	{
		std::string s = salt();
		return encrypt(clear,salt);
	}

	std::string encrypt(const std::string& clear, const std::string& salt)
	{
		cd_.initialized = 0;
		char* c = crypt_r( clear.c_str(), salt.c_str(), &cd_);
		return c;
	}

	bool check( const std::string& clear, const std::string& hash)
	{
		std::string c = encrypt(clear,hash);
		return hash == c;
	}

	std::string salt()
	{
		std::string n = nonce(43);
		std::string salt = std::string("$5$") + base64_encode((unsigned char*)n.c_str(),n.size()).substr(0,43) + "$";
		return salt;
	}

private:

	crypt_data cd_;

};
*/
/*

TEST_F(BasicTest, SimpleCrypt) {

	std::string n = nonce(43);
	std::string salt = std::string("$5$") + base64_encode((unsigned char*)n.c_str(),n.size()).substr(0,43) + "$";
	std::string clear = "secret_pwd";

	crypt_data cd;
	cd.initialized = 0;

	char* c = crypt_r( clear.c_str(), salt.c_str(), &cd);

	if(c == 0)
	{
		std::cout << "c is null" << errno << std::endl;
	}

	std::cout << clear << std::endl << salt << std::endl << c << std::endl << cd.keysched << std::endl;
}
*/

int main(int argc, char **argv) {

	prio::Libraries<prio::EventLoop> init;

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

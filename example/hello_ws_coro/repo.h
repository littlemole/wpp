#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "reproredis/redis.h"
#include "reprosqlite/sqlite.h"
#include "reproweb/json/json.h"
#include "model.h"


class UserRepository
{
public:

	UserRepository(std::shared_ptr<reprosqlite::SqlitePool> sqlitePool)
		: sqlite(sqlitePool)
	{}

	Future<User> register_user( 
		const std::string& username, 
		const std::string& login, 
		const std::string& pwd, 
		const std::string& avatar_url )
	{
		try
		{
			if(username.empty() || login.empty() || pwd.empty())
			{
				throw RegistrationEx("username, login and password may not be empty");
			}

			cryptoneat::Password pass;
			std::string hash = pass.hash(pwd);

			User result(username,login,hash,avatar_url);

			try
			{		 
				reprosqlite::Result r = co_await sqlite->query(
					"INSERT INTO users (username,login,pwd,avatar_url) VALUES ( ? , ? , ? , ? )",
					username,login,hash,avatar_url
				);

			}
			catch(const std::exception& ex)
			{
				throw RegistrationEx("error.msg.login.alreaady.taken");
			}

			co_return result;
		}
		catch(const std::exception& ex)
		{
			throw RegistrationEx(ex.what());
		}
	}

	Future<User> get_user( const std::string& login )
	{
		try
		{
			reprosqlite::Result r = co_await sqlite->query(
				"SELECT username,login,pwd,avatar_url FROM users WHERE login = ? ;",
				login
			);

			if ( r.rows() < 1) throw repro::Ex("user not found");

			User result(
				r[0][0],
				r[0][1],
				r[0][2],
				r[0][3]
			);

			co_return result;
		}
		catch(const std::exception& ex)
		{
			throw LoginEx("error.msg.login.failed");
		}
	}

private:

	std::shared_ptr<reprosqlite::SqlitePool> sqlite;
};

 


struct UserPool : public reprosqlite::SqlitePool
{
	UserPool(std::shared_ptr<Config> config) 
	  : SqlitePool(config->getString("sqlite")) 
	{}
};

#endif

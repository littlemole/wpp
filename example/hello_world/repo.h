#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "reproredis/redis.h"
#include "reprosqlite/sqlite.h"
#include "reproweb/json/json.h"
#include "model.h"

using namespace prio;
using namespace repro;


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
		auto p = repro::promise<User>();

		if(username.empty() || login.empty() || pwd.empty())
		{
			return p.rejected(RegistrationEx("username, login and password may not be empty"));
		}

		cryptoneat::Password pass;
		std::string hash = pass.hash(pwd);

		User result(username,login,hash,avatar_url);
		 
		sqlite->query(
			"INSERT INTO users (username,login,pwd,avatar_url) VALUES ( ? , ? , ? , ? )",
			username,login,hash,avatar_url
		)
		.then( [p,result](reprosqlite::Result r) 
		{
			p.resolve(result);
		})
		.otherwise( [p](const std::exception& ex)
		{
			p.reject(RegistrationEx("login is already taken"));
		});

		return p.future();
	}

	Future<User> get_user( const std::string& login )
	{
		auto p = repro::promise<User>();

		sqlite->query(
			"SELECT username,login,pwd,avatar_url FROM users WHERE login = ? ;",
			login
		)
		.then( [p](reprosqlite::Result r) 
		{
			if ( r.rows() < 1) throw repro::Ex("user login not found");

			User result(
				r[0][0],
				r[0][1],
				r[0][2],
				r[0][3]
			);

			p.resolve(result);
		})
		.otherwise( [p](const std::exception& ex)
		{
			p.reject(AuthEx("invalid credentials specified. pls try again."));
		});

		return p.future();
	}

private:

	std::shared_ptr<reprosqlite::SqlitePool> sqlite;
};


#endif

#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "repromysql/mysql-async.h"
#include "entities.h"

using namespace prio;
using namespace repro;
using namespace reproweb;


class UserRepository
{
public:

	UserRepository(std::shared_ptr<repromysql::MysqlPool> my)
		: mysql(my)
	{}

	Future<> register_user( User user )
	{
		auto p = promise<>();

		if(user.username().empty() || user.login().empty() || user.hash().empty())
		{
			throw BadRequestEx("username, login and password may not be empty");
		}

		cryptoneat::Password pass;
		std::string hash = pass.hash(user.hash());

		try
		{
			repromysql::mysql_async::Ptr r = co_await mysql->execute(
						"INSERT INTO users (username,email,pwd,avatar_url) VALUES ( ? , ? , ? , ? )",
						user.username(),user.login(),hash,user.avatar_url()
			);
		}
		catch(const std::exception& ex)
		{
			std::cout << "register failed: " << ex.what() << std::endl;
			throw LoginAlreadyTakenEx("error.msg.login.already.taken");
		}

		co_return;
	}

	Future<User> get_user( const std::string& login )
	{
		auto p = promise<User>();

		try
		{
			repromysql::result_async::Ptr r = co_await mysql->query(
					"SELECT username,email,pwd,avatar_url FROM users WHERE email = ? ;",
					login
			);

			if( r->fetch() )
			{
				User result( 
					r->field(0).getString(), 
					r->field(1).getString(), 
					r->field(2).getString(),
					r->field(3).getString()
				);

				co_return result;
			}
			else
			{
				throw UserNotFoundEx("error.msg.login.failed");
			}		
		}
		catch(const std::exception& ex)
		{
			 throw UserNotFoundEx("error.msg.login.failed");
		}
		co_return User();
	}

private:

	std::shared_ptr<repromysql::MysqlPool> mysql;
};

struct UserPool : public repromysql::MysqlPool
{
	UserPool(std::shared_ptr<Config> config) 
	  : MysqlPool(config->getString("mysql")) 
	{}
};

#endif

#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_REPO_DEFINE_

#include "repromysql/mysql-async.h"
#include "entities.h" 
 
using namespace prio;
using namespace repro;
using namespace reproweb;

class UserMysqlRepository
{
public:

	UserMysqlRepository(std::shared_ptr<repromysql::MysqlPool> mp)
		: mysql(mp)
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

		mysql->execute(
					"INSERT INTO users (username,email,pwd,avatar_url) VALUES ( ? , ? , ? , ? )",
					user.username(),user.login(),hash,user.avatar_url()
		)
		.then([p,user](repromysql::mysql_async::Ptr)
		{
			p.resolve();
		})
		.otherwise([p](const std::exception& ex)
		{
			std::cout << "register failed: " << ex.what() << std::endl;
			p.reject(LoginAlreadyTakenEx("error.msg.login.already.taken"));
		});

		return p.future();
	}

	Future<User> get_user( const std::string& login )
	{
		auto p = promise<User>();

		mysql->query(
				"SELECT username,email,pwd,avatar_url FROM users WHERE email = ? ;",
				login
		)
		.then([p](repromysql::result_async::Ptr r)
		{
			if( r->fetch() )
			{
				User result( 
					r->field(0).getString(), 
					r->field(1).getString(), 
					r->field(2).getString(),
					r->field(3).getString()
				);

				p.resolve(result);
			}
			else
			{
				throw UserNotFoundEx("error.msg.login.failed");
			}
		})
		.otherwise([p](const std::exception& ex)
		{
			p.reject(UserNotFoundEx("error.msg.login.failed"));
		});

		return p.future();
	}

private:

	std::shared_ptr<repromysql::MysqlPool> mysql;
};


struct UserMysqlPool : public repromysql::MysqlPool 
{
	UserMysqlPool(std::shared_ptr<Config> config) 
	  : MysqlPool(config->getString("mysql")) 
	{}
};

#endif

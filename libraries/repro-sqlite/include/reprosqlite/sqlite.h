#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SQLITE_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SQLITE_DEF_GUARD_

#include "reprocpp/promise.h"

#include <sqlite3.h>
#include <set>
//////////////////////////////////////////////////////////////


namespace reprosqlite {

class Statement;
class Result;

class SqlitePool
{
public:

	SqlitePool(const std::string& db, int capacity = 1);
	~SqlitePool();

	sqlite3* getConnection();
	void returnConnection(sqlite3* r);
	void returnErroredConnection(sqlite3* r);

	std::shared_ptr<Statement> stm(const std::string& sql);

	template<class ... Args>
	repro::Future<Result> query(const std::string& sql, Args ... args);

private:

	template<class T, class ... Args>
	void bind(std::shared_ptr<Statement>& st, T t, Args ... args);

	void bind(std::shared_ptr<Statement>& st)
	{}

	std::string host_;
	unsigned int capacity_;
	std::set<sqlite3*> used_;
	std::set<sqlite3*> unused_;
};


class Result
{
public:

	Result() 
	{
		REPRO_MONITOR_INCR(sqliteResult);
	}

	Result(const Result& rhs)
		:data(rhs.data),columns(rhs.columns)
	{
		REPRO_MONITOR_INCR(sqliteResult);
	}

	Result(Result&& rhs)
		: data(std::move(rhs.data)),
		columns(std::move(rhs.columns))
	{
		REPRO_MONITOR_INCR(sqliteResult);
	}
	
	~Result()
	{
		REPRO_MONITOR_DECR(sqliteResult);	
	}

	sqlite3_int64 last_insert_id;

	unsigned int rows() { return data.size(); }
	unsigned int cols()
	{
		if ( data.empty() ) return 0;
		return data[0].size();
	}

	std::vector<std::string>& row(unsigned int i)
	{
		return data[i];
	}

	std::vector<std::string>& operator[](unsigned int i)
	{
		return data[i];
	}

	std::vector<std::vector<std::string>> data;
	std::vector<std::string> columns;

private:

	Result& operator=(const Result&) = delete;
	Result& operator=(Result&&) = delete;

};


class Statement : public std::enable_shared_from_this<Statement>
{
public:

	typedef repro::Future<Result> FutureType;
	typedef std::shared_ptr<Statement> Ptr;

	~Statement();

	static Ptr create(SqlitePool& pool);

	Statement::Ptr stmt(const std::string& sql);
	Statement::Ptr bind(const std::string& val);
	FutureType exec();

private:

	Statement(SqlitePool& pool);
	Statement(const Statement&) = delete;
	Statement& operator=(const Statement&) = delete;

	std::string sql_;
	std::vector<std::string> values_;

	SqlitePool& pool_;
	sqlite3* sqlite3_;

	Ptr self_;
};


template<class ... Args>
repro::Future<Result> SqlitePool::query(const std::string& sql, Args ... args)
{
	auto st = stm(sql);
	bind(st, args...);
	return st->exec();
}

template<class T, class ... Args>
void SqlitePool::bind(std::shared_ptr<Statement>& st, T t, Args ... args)
{
	std::ostringstream oss;
	oss << t;
	st->bind(oss.str());
	bind(st, args...);
}

} // end namespace sqlite

#endif


#include <sstream>
#include <fstream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <atomic>

#include <reprosqlite/sqlite.h>
#include <priocpp/task.h>

using namespace prio;


namespace reprosqlite {

SqlitePool::SqlitePool(const std::string& host,  int capacity)
	: host_(host), capacity_(capacity)
{}

SqlitePool::~SqlitePool()
{
	for( auto i : used_)
	{
		sqlite3_close(i);
	}
	for( auto i : unused_)
	{
		sqlite3_close(i);
	}
	used_.clear();
	unused_.clear();
}

sqlite3* SqlitePool::getConnection()
{
	if(!unused_.empty())
	{
		sqlite3* r = *(unused_.begin());
		unused_.erase(r);
		used_.insert(r);
		return r;
	}

	sqlite3* r = 0;
    int rc = sqlite3_open(host_.c_str(), &r);
    if( rc )
    {
    	throw repro::Ex("cannot open db");
    }

	used_.insert(r);
	return r;
}

void SqlitePool::returnConnection(sqlite3* r)
{
	if (used_.find(r) != used_.end() )
	{
		used_.erase(r);
		unused_.insert(r);
		while(unused_.size() > capacity_)
		{
			sqlite3* tmp = *(unused_.begin());
			unused_.erase(tmp);
			sqlite3_close(tmp);
		}
	}
	else
	{
		sqlite3_close(r);
	}
}


void SqlitePool::returnErroredConnection(sqlite3* r)
{
	if (used_.find(r) != used_.end() ) {
		used_.erase(r);
	}
	sqlite3_close(r);
}

std::shared_ptr<Statement> SqlitePool::stm(const std::string& sql)
{
	return reprosqlite::Statement::create(*this)
	->stmt(sql);
}


///////////////////////////////////////////////////////////////


Statement::~Statement()
{
	pool_.returnConnection(sqlite3_);
	REPRO_MONITOR_DECR(sqliteStatement);
}

Statement::Ptr Statement::create(SqlitePool& pool)
{
	auto that = std::shared_ptr<Statement>(new Statement(pool));
	return that;
}

Statement::Ptr Statement::stmt(const std::string& sql)
{
	sql_ = sql;
	return shared_from_this();
}

Statement::Ptr Statement::bind(const std::string& val)
{
	values_.push_back(val);
	return shared_from_this();
}

Statement::FutureType Statement::exec(  )
{
	auto p = repro::promise<Result>();

	self_ = shared_from_this();

	task( [this]
	{
		sqlite3_stmt *stm;

		int rc = sqlite3_prepare_v2(sqlite3_, sql_.c_str(), -1, &stm, 0);

		if (rc != SQLITE_OK)
		{
			throw repro::Ex("create stm failed");
		}

		Result result;

		for ( unsigned int i = 0; i < values_.size(); i++)
		{
			sqlite3_bind_text(stm, i+1, values_[i].c_str(), (int) values_[i].size(), SQLITE_TRANSIENT);
		}

		int step = sqlite3_step(stm);

		if(step!=SQLITE_OK && step<SQLITE_ROW)
		{
			sqlite3_finalize(stm);
			throw repro::Ex("execute statement failed");
		}

		int n = sqlite3_column_count(stm);
		for ( int i = 0; i < n; i++ )
		{
			const char* colname = sqlite3_column_name(stm,i);
			result.columns.push_back(std::string(colname));
		}

		while (step == SQLITE_ROW)
		{
			int n = sqlite3_column_count(stm);
			std::vector<std::string> v;
			for ( int i = 0; i < n; i++ )
			{
				v.push_back(std::string((char*)sqlite3_column_text(stm, i)));
			}
			result.data.push_back(std::move(v));
			step = sqlite3_step(stm);
		}

		sqlite3_finalize(stm);

		if(step==SQLITE_OK || step == SQLITE_DONE)
		{
			result.last_insert_id = sqlite3_last_insert_rowid(sqlite3_);
			return result;
		}

		throw repro::Ex("execute statement failed");
	})
	.then([this,p](Result r)
	{
		auto tmp = self_;
		self_.reset();
		p.resolve(r);
	})
	.otherwise( [this,p] (const std::exception_ptr& ex)
	{
		self_.reset();
		p.reject(ex);
	});

	return p.future();
}


Statement::Statement(SqlitePool& pool)
	: pool_(pool),
	  sqlite3_(pool.getConnection())
{
		REPRO_MONITOR_INCR(sqliteStatement);
}


}



#ifndef INCLUDE_PROMISE_WEB_CONFIG_H_
#define INCLUDE_PROMISE_WEB_CONFIG_H_

//! \file config.h

#include "reproweb/json/json.h"
#include "priohttp/common.h"

namespace reproweb  {

//! generic JSON based Configuration
//! \ingroup webserver

class Config
{
public:

	//! construct empty config
	Config()
	{}

	//! construct Config from JSON file specified by path
	Config( const std::string& path)
	{
		load(path);
	}

	//! load config from JSON at path
	Config& load( const std::string& path)
	{
		std::string f = prio::slurp(path);
		json_ = JSON::parse(f);
		return *this;
	}

	//! return the config JSON
	Json::Value& json()
	{
		return json_;
	}

	//! get a simple value from JSON as Json object
	Json::Value& get(const std::string& key)
	{
		return json_[key];
	}

	//! get a simple value from config as string
	std::string getString(const std::string& key)
	{
		return json_[key].asString();
	}

	//! get a simple value from config as int
	int getInt(const std::string& key)
	{
		return json_[key].asInt();
	}

private:

	Json::Value json_;
};


}


#endif /* INCLUDE_PROMISE_WEB_CONFIG_H_ */

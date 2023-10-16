#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VIEW_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VIEW_DEFINE_

#include <string>
#include <memory>

#include "reproweb/tools/config.h"
#include "reproweb/view/tpl.h"

using namespace prio;

class View
{
public:

	View(std::shared_ptr<reproweb::Config> conf)
		: config_(conf)
	{
		templates_.load("/view/");
	}

	void render_index(Response& res, Json::Value json)
	{
		json["version"] = config_->getString("version");
		res
		.body(templates_.render("index", json ))
		.contentType("text/html")
		.ok()
		.flush();
	}

	void render_login(Response& res, const std::string& msg )
	{
		Json::Value json(Json::objectValue);
		json["errorMsg"] = msg;
		json["version"] = config_->getString("version");

		res
		.body(templates_.render("login", json ))
		.contentType("text/html")
		.ok()
		.flush();
	}

	void render_registration(Response& res, const std::string& msg )
	{
		Json::Value json(Json::objectValue);
		json["errorMsg"] = msg;
		json["version"] = config_->getString("version");

		res
		.body(templates_.render("register", json ))
		.contentType("text/html")
		.ok()
		.flush();
	}	

	void redirect_to_index(Response& res)
	{
		res
		.redirect("https://localhost:9876/#")
		.flush();
	}

	void redirect_to_login(Response& res)
	{
		res
		.redirect("https://localhost:9876/login")
		.flush();
	}	

private:
	reproweb::TplStore templates_;
	std::shared_ptr<reproweb::Config> config_;
};
 
#endif


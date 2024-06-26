#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VIEW_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VIEW_DEFINE_

#include "reproweb/tools/config.h"
#include "reproweb/view/tpl.h"
#include "reproweb/view/i18n.h"
#include "reproweb/ctrl/ssi.h"

using namespace prio;
using namespace repro;
using namespace reproweb;

class View
{
public:

	View(
		std::shared_ptr<AppConfig> config, 
		std::shared_ptr<TplStore> tpls, 
		std::shared_ptr<I18N> i18n )
		: templates_(tpls), i18n_(i18n)
	{
		version_ = config->getString("version");
	}

	void render_index(Request& req, Response& res, Json::Value profile)
	{
		render(req,res,"index",profile);
	}

	void render_login(Request& req, Response& res, const std::string& errMsg )
	{
		render(req,res,"login",errMsg);
	}

	void render_registration(Request& req, Response& res, const std::string& errMsg )
	{
		render(req,res,"register",errMsg);
	}	

	void render_error(const std::exception& ex, Response& res)
	{
		std::ostringstream oss;
		oss << typeid(ex).name() << ":" << ex.what() << std::endl;

		res
		.body(oss.str())
		.error()
		.flush();
	}

	void redirect_to_index(Request& req, Response& res)
	{
		res
		.redirect(req,"/")
		.flush();
	}

	void redirect_to_login(Request& req, Response& res)
	{
		res
		.redirect(req,"/login")
		.flush();
	}	

	void redirect_to_registration(Request& req, Response& res)
	{
		res
		.redirect(req,"/register")
		.flush();
	}	

private:

	std::shared_ptr<TplStore> templates_;
	std::shared_ptr<I18N> i18n_;
	std::string version_;

	void render(Request& req, Response& res, const std::string& page, const std::string& errMsg)
	{
		return render(req,res,page,error_msg(get_locale(req),errMsg));
	}

	void render(Request& req, Response& res, const std::string& page, Json::Value value)
	{
		value["page"] = page;
		value["version"] = version_;

		std::string locale = get_locale(req);
		std::string view = templates_->get(page);

		SSIResolver::resolve(req,view)
		.then( [this,&res,value,locale](std::string txt)
		{
			std::string tmpl = i18n_->render(locale,txt);

			std::map<std::string,mustache::Data> partials;
			partials["header"] = templates_->get("header");
			partials["partials/header"] = templates_->get("partials/header");

			std::map<std::string,std::function<std::string(const std::string&)>> lambdas;

			lambdas["i18n"] = [this,locale]( const std::string& key)
			{
				std::cout << "i18n: " << key << std::endl;
				return i18n_->key(locale,key);
			};

			std::string content = mustache::render(tmpl,value,partials,lambdas);

			res
			.body(content)
			.contentType("text/html")
			.ok()
			.flush();
		})
		.otherwise([&res](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			res.error().flush();
		});
	}

	Json::Value error_msg(const std::string& locale, const std::string& msg )
	{
		Json::Value errorMsg(Json::objectValue);
		errorMsg["errorMsg"] = "";

		if(!msg.empty())
		{
			errorMsg["errorMsg"] = i18n_->key(locale,msg);
		}

		return errorMsg;
	}	

	static std::string get_locale(Request& req)
	{
		auto h = req.headers.values("Accept-Language");
		auto lang = h.value().main();
		std::string locale = std::regex_replace (lang,std::regex("-"),"_");		

		return locale;
	}  	
};
 
#endif


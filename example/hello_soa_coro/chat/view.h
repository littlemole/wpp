#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VIEW_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VIEW_DEFINE_

#include "reproweb/ctrl/ssi.h"
#include "reproweb/view/i18n.h"
#include "reproweb/view/tpl.h"


class View : public I18nSSIMustacheView
{
public:

	View(
		std::shared_ptr<AppConfig> conf, 
		std::shared_ptr<TplStore> tpls, 
		std::shared_ptr<I18N> i18n )
		: I18nSSIMustacheView(tpls,i18n),i18n_(i18n)
	{
		version_ = conf->getString("version");
	}

	Future<std::string> render_index(Request& req, Json::Value value)
	{
		value["page"] = "index";
		value["version"] = version_;

		return render_content(req,"index",value);
	}

	Future<std::string> render_login(Request& req,const std::string& errMsg )
	{
		Json::Value value = error_msg(get_locale(req),errMsg);
		value["page"] = "login";
		value["version"] = version_;

		return render_content(req,"login",value);
	}

	Future<std::string> render_registration(Request& req, const std::string& errMsg )
	{
		Json::Value value = error_msg(get_locale(req),errMsg);
		value["page"] = "register";
		value["version"] = version_;

		return render_content(req,"register",value);
	}	

	void redirect_to_index(Request& req, Response& res, const std::string& sid)
	{
		redirect( req, res.cookie(Cookie("repro_web_sid", sid)), "/" );
	}

	void redirect_to_login(Request& req, Response& res)
	{
		redirect(req, res,"/login");
	}	

	void redirect_to_registration(Request& req, Response& res)
	{
		redirect(req, res,"/register");
	}	

private:

	std::shared_ptr<I18N> i18n_;
	std::string version_;

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

};
 
#endif


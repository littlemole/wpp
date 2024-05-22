#ifndef INCLUDE_CSGI_MUSTACHE_TPL_H_
#define INCLUDE_CSGI_MUSTACHE_TPL_H_

//! \file tpl.h
//! \defgroup view

#include "priocpp/api.h"
#include "reproweb/json/json.h"
#include "reproweb/view/i18n.h"
#include "reproweb/ctrl/ssi.h"
#include <mustache.hpp>

namespace reproweb  {

//////////////////////////////////////////////////////

//! Mustache wrapper
//! \ingroup view
class mustache
{
public:

	using Mustache = kainjow::mustache::mustache;//<std::string>;
	using Data = kainjow::mustache::data;

	//! constrcut mustache template from string
	mustache( const std::string& tpl);

	void add_partial(const std::string& name, const std::string& tpl);
	void add_lambda(const std::string& name, std::function<std::string(std::string)>);

	//! \private
	std::string render(Data& data);
	//! render template with data from JSON
	std::string render(Json::Value data);

	//! \private
	static std::string render(const std::string& tpl,Data& data);
	//! render template with data from JSON
	static std::string render(const std::string& tpl,Json::Value data);

	static std::string render(const std::string& tpl,Data& data, const std::map<std::string,Data> partials);
	//! render template with data from JSON
	static std::string render(const std::string& tpl,Json::Value data, const std::map<std::string,Data> partials);

	static std::string render(const std::string& tpl,Data& data, const std::map<std::string,Data> partials,std::map<std::string,std::function<std::string(const std::string&)>> lambdas);
	//! render template with data from JSON
	static std::string render(const std::string& tpl,Json::Value data, const std::map<std::string,Data> partials, std::map<std::string,std::function<std::string(const std::string&)>> lambdas);

	//! \private
	static Data fromJson(Json::Value& data);

private:
	std::map<std::string,std::function<std::string(const std::string&)>> lambdas_;
	std::map<std::string,Data> partials_;
	std::string template_;
};

//! Mustache Template Store
//! \ingroup view

class TplStore
{
public:

	//! construct empty template store
	TplStore();

	//! construct empty template store with i18n support
	TplStore(std::shared_ptr<I18N> i18n) ;

	//! register a stemplate with store by name
	void register_tpl(const std::string& name, const std::string& tpl);
	//! unregister a template
	void unregister_tpl(const std::string& name);
	//! get template by name
	std::string& get(const std::string& name);

	bool exists(const std::string& name);

	//! load all templates from given directory
	void load(const std::string& path);

	std::vector<std::string> keys();

	//! render named template with given data from JSON
	std::string render(const std::string& tpl, Json::Value val);

	//! render named template with given data from JSON and locale
	std::string render(const std::string& tpl, const std::string& locale, Json::Value val);

	std::string render(const std::string& tpl, Json::Value val, const std::vector<std::string>& partials);
	std::string render(const std::string& tpl, const std::string& locale, Json::Value val, const std::vector<std::string>& partials);

	//! \private
	// same as above, just with JSON proved as string instead of JSON object
	std::string render(const std::string& tpl, const std::string& json);
	std::string render(const std::string& tpl, const std::string& json, const std::vector<std::string>& partials);

	std::string render(const std::string& tpl, const std::string& locale, const std::string& json);
	std::string render(const std::string& tpl, const std::string& locale, const std::string& json, const std::vector<std::string>& partials);

	std::shared_ptr<I18N> i18n()
	{
	    return i18n_;
	}
private:

	void doload(const std::string& base, const std::string& path);

	std::shared_ptr<I18N> i18n_;
	std::string path_;
	std::map<std::string,std::string> templates_;
};

//! declare views with mustache templates in WebApplicationContext
//! \ingroup view

class view_templates
{
public:

	typedef view_templates type;

	//! construct views from template files at directory
    view_templates(const std::string& path);

	//! \private
    void ctx_register(diy::Context* ctx);

private:

    std::string path_;
};

//////////////////////////////////////////////////////

//! abstract view base class
//! \ingroup view

class AbstractView
{
public:

	AbstractView();
	virtual ~AbstractView();

	//! get locale from request
	static std::string get_locale(prio::Request& req);

	//! render given page with JSON data specified and flush() the response
	virtual repro::Future<> render(prio::Request& req, prio::Response& res, const std::string& page, Json::Value value);

	//! render given page with JSON data and return template as string
	virtual repro::Future<std::string> render_content(prio::Request& req, const std::string& page, Json::Value value) = 0;

	//! generate absolute HTTP redirect response
	void redirect(prio::Response& res, const std::string& url);
	//! generate relative HTTP redirect response
	void redirect(prio::Request& req, prio::Response& res, const std::string& url);
	
	//! flush content
	void flush_content(prio::Response& res,const std::string& content);
	//! render error
	void render_error(prio::Response& res, const std::exception& ex);
};


/*

	AbstractView
		TemplateView
			LayoutView
				MustacheView (with partials and i18n lambda)
*/

//! Mustache Template view
//! \ingroup view


class TemplateView : public AbstractView
{
public:

	TemplateView( std::shared_ptr<TplStore> tpls);

	virtual repro::Future<std::string> render_content(prio::Request& req, const std::string& page, Json::Value value);

protected:

	std::shared_ptr<TplStore> templates_;
};

//! I18n View decorator
//! \ingroup view

class ViewI18nDecorator : public AbstractView
{
public:

	ViewI18nDecorator( std::shared_ptr<I18N> i18n,  AbstractView* av);

	virtual repro::Future<std::string> render_content(prio::Request& req, const std::string& page, Json::Value value);

protected:
	std::shared_ptr<I18N> i18n_;
	std::unique_ptr<AbstractView> view_;
};


//! Mustache View Decorator
//! \ingroup view

class ViewMustacheDecorator : public AbstractView
{
public:

	ViewMustacheDecorator(  AbstractView* av);

	virtual repro::Future<std::string> render_content(prio::Request& req, const std::string& page, Json::Value value);

protected:
	std::unique_ptr<AbstractView> view_;
};

//! SSI View Decorator
//! \ingroup view

class ViewSSIDecorator : public AbstractView
{
public:

	ViewSSIDecorator( AbstractView* av);

	virtual repro::Future<std::string> render_content(prio::Request& req, const std::string& page, Json::Value value);

protected:
	std::unique_ptr<AbstractView> view_;
};

//////////////////////////////////////////////////////

//! I18nSSIMustache View - view that does i18n and SSI and Mustache
//! \ingroup view

class I18nSSIMustacheView : public ViewMustacheDecorator
{
public:

	I18nSSIMustacheView( std::shared_ptr<TplStore> tpls, std::shared_ptr<I18N> i18n );
};

//! I18nMustacheView - view that does i18n and mustache
//! \ingroup view

class I18nMustacheView : public ViewMustacheDecorator
{
public:

	I18nMustacheView( std::shared_ptr<TplStore> tpls, std::shared_ptr<I18N> i18n );
};

//! MustacheView - only Mustache
//! \ingroup view

class MustacheView : public ViewMustacheDecorator
{
public:

	MustacheView( std::shared_ptr<TplStore> tpls );
};

//! SSIMustacheView - only Mustache and SSI
//! \ingroup view
class SSIMustacheView : public ViewMustacheDecorator
{
public:

	SSIMustacheView( std::shared_ptr<TplStore> tpls );
};


//////////////////////////////////////////////////////

class Autohandler
{
public:
	Autohandler(std::shared_ptr<TplStore> tpls) : templates_(tpls)
	{}

	std::string expand(const std::string page)
	{
		static std::regex rgx("<!--#autohandler -->");

		std::string ah = templates_->get("autohandler");
		std::cout << "AH:" << ah << std::endl;

		std::string view = templates_->get(page);
		std::string content = std::regex_replace (ah,rgx,view);		
		std::cout << "AH-X:" << content << std::endl;
		return content;
	}

private:
	std::shared_ptr<TplStore> templates_;

};


class AutohandlerView : public AbstractView
{
public:
	AutohandlerView(std::shared_ptr<TplStore> tpls) 
	  : autohandler_(tpls), templates_(tpls)
	{}

	
	virtual repro::Future<std::string> render_content(prio::Request& /*req*/, const std::string& page, Json::Value value)
	{
		auto p = repro::promise<std::string>();

		std::string ah = templates_->get("autphandler");
		std::cout << "AH:" << ah << std::endl;

		prio::nextTick( [this,p,page,value,ah]() 
		{
			std::string view = autohandler_.expand(page);
			p.resolve(view);
		});

		return p.future();

	}

private:
    Autohandler autohandler_;
	std::shared_ptr<TplStore> templates_;

};


}

#endif /* INCLUDE_CSGI_MUSTACHE_TPL_H_ */

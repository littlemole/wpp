#include "reproweb/view/tpl.h"
#include "priohttp/common.h"
#include <regex>

namespace reproweb  {

//////////////////////////////////////////////////////

mustache::mustache( const std::string& tpl)
	: template_(tpl)
{}

std::string mustache::render(Json::Value data)
{
	return render(template_,data);
}


std::string mustache::render(Data& data)
{
	return render(template_,data);
}

std::string mustache::render(const std::string& tpl, Json::Value data)
{
	Data d(fromJson(data));
	return render(tpl,d);
}


std::string mustache::render(const std::string& tpl, Data& data)
{
	std::ostringstream oss;

	Mustache tmpl{tpl};

	tmpl.render(data, oss);

	return oss.str();
}

mustache::Data mustache::fromJson(Json::Value& data)
{
	if ( data.isArray())
	{
		Data d(Data::type::list);
		for( unsigned int i = 0; i < data.size(); i++)
		{
			d.push_back( fromJson( data[i]));
		}
		return d;
	}
	if ( data.isObject())
	{
		Data d(Data::type::object);
		std::vector<std::string> members = data.getMemberNames();
		for( std::string m : members)
		{
			d.set(m, fromJson( data[m]));
		}
		return d;
	}
	if ( data.isNull())
	{
		return Data(Data::type::bool_false);
	}
	return Data(data.asString());
}

//////////////////////////////////////////////////////


TplStore::TplStore()
{
	path_ = prio::get_current_work_dir();
}

void TplStore::register_tpl(const std::string& name, const std::string& tpl)
{
	templates_[name] = tpl;
}

void TplStore::unregister_tpl(const std::string& name)
{
	templates_.erase(name);
}

std::string& TplStore::get(const std::string& name)
{
	return templates_[name];
}

void TplStore::load(const std::string& path)
{
	std::string p = path_ + path;
	p = prio::real_path(p);

	std::vector<std::string> v = prio::glob(p);
	for ( std::string s : v )
	{
		std::string fn = p + "/" + s;
		std::string f = prio::slurp(fn);
		if ( f.empty() )
		{
			continue;
		}
		std::string n = s;
		size_t pos = s.find_last_of(".");
		if ( pos != std::string::npos)
		{
			n = s.substr(0,pos);
		}
		register_tpl(n,f);
	}
}



std::string TplStore::render(const std::string& tpl, Json::Value val)
{
	mustache m = {
		get(tpl)
	};

	return m.render(val);
}

std::string TplStore::render(const std::string& tpl, const std::string& json)
{
	mustache m = {
		get(tpl)
	};

	Json::Value val = JSON::parse(json);
	return m.render(val);
}

//////////////////////////////////////////////////////


view_templates::view_templates(const std::string& path)
	: path_(path)
{}

void view_templates::ctx_register(diy::Context* ctx)
{
	auto tpls = std::make_shared<TplStore>();
	tpls->load(path_);
	ctx->register_static<TplStore>(tpls);
}

//////////////////////////////////////////////////////


AbstractView::AbstractView( )
{}

AbstractView::~AbstractView( )
{}

std::string AbstractView::get_locale(prio::Request& req)
{
	auto h = req.headers.values("Accept-Language");
	auto lang = h.value().main();
	std::string locale = std::regex_replace (lang,std::regex("-"),"_");		

	return locale;
}  	

void AbstractView::redirect(prio::Response& res, const std::string& url)
{
	res.redirect(url).flush();
}

void AbstractView::redirect(prio::Request& req, prio::Response& res, const std::string& url)
{
	res.redirect(req,url).flush();
}

repro::Future<> AbstractView::render(prio::Request& req, prio::Response& res, const std::string& page, Json::Value value)
{
	auto p = repro::promise<>();

	this->render_content(req,page,value)
	.then([this,p,&res](std::string content)
	{
		flush_content(res,content);
	})
	.otherwise([this,p,&res](const std::exception& ex)
	{
		render_error(res,ex);
	});

	return p.future();
}

void AbstractView::flush_content(prio::Response& res,const std::string& content)
{
	res
	.body(content)
	.contentType("text/html")
	.ok()
	.flush();
}

void AbstractView::render_error(prio::Response& res, const std::exception& ex)
{
	std::ostringstream oss;
	oss << typeid(ex).name() << ":" << ex.what() << std::endl;

	res
	.body(oss.str())
	.error()
	.flush();
}	

//////////////////////////////////////////////////////

TemplateView::TemplateView( std::shared_ptr<TplStore> tpls)
	: templates_(tpls)
{}

repro::Future<std::string> TemplateView::render_content(prio::Request& req, const std::string& page, Json::Value value)
{
	auto p = repro::promise<std::string>();

	prio::nextTick( [this,p,page,value]() 
	{
		// fetch template for page
		std::string view = templates_->get(page);

		p.resolve(view);
	});

	return p.future();
}

//////////////////////////////////////////////////////


ViewI18nDecorator::ViewI18nDecorator( std::shared_ptr<I18N> i18n,  AbstractView* av)
	: i18n_(i18n),view_(av)
{}

repro::Future<std::string> ViewI18nDecorator::render_content(prio::Request& req, const std::string& page, Json::Value value)
{
	auto p = repro::promise<std::string>();

	view_->render_content(req,page,value)
	.then([this,p,&req](std::string content)
	{
		std::string tmpl = i18n_->render(get_locale(req),content);

		p.resolve(tmpl);
	})
	.otherwise(reject(p));
	
	return p.future();
}

//////////////////////////////////////////////////////


ViewMustacheDecorator::ViewMustacheDecorator(  AbstractView* av)
	: view_(av)
{}

repro::Future<std::string> ViewMustacheDecorator::render_content(prio::Request& req, const std::string& page, Json::Value value)
{
	auto p = repro::promise<std::string>();

	view_->render_content(req,page,value)
	.then([p,value](std::string view)
	{
		std::string content = mustache::render(view,value);

		p.resolve(content);
	})
	.otherwise(reject(p));
	
	return p.future();
}

//////////////////////////////////////////////////////


ViewSSIDecorator::ViewSSIDecorator( AbstractView* av)
	: view_(av)
{}

repro::Future<std::string> ViewSSIDecorator::render_content(prio::Request& req, const std::string& page, Json::Value value)
{
	auto p = repro::promise<std::string>();

	view_->render_content(req,page,value)
	.then([&req](std::string content)
	{
		return SSIResolver::resolve(req,content);
	})
	.then([p](std::string txt)
	{
		p.resolve(txt);
	})		
	.otherwise(reject(p));
	
	return p.future();
}

//////////////////////////////////////////////////////


I18nSSIMustacheView::I18nSSIMustacheView(	
	std::shared_ptr<TplStore> tpls, 
	std::shared_ptr<I18N> i18n )
	: ViewMustacheDecorator( 
		new ViewI18nDecorator(
			i18n,
			new ViewSSIDecorator(
				new TemplateView(tpls)
			)
		)
	)
{}

I18nMustacheView::I18nMustacheView(	
	std::shared_ptr<TplStore> tpls, 
	std::shared_ptr<I18N> i18n )
	: ViewMustacheDecorator( 
		new ViewI18nDecorator(
			i18n,
			new TemplateView(tpls)
		)
	)
{}

MustacheView::MustacheView( std::shared_ptr<TplStore> tpls )
	: ViewMustacheDecorator( 
		new TemplateView(tpls)
	)
{}

SSIMustacheView::SSIMustacheView( std::shared_ptr<TplStore> tpls )
	: ViewMustacheDecorator( 
		new ViewSSIDecorator(
			new TemplateView(tpls)
		)
	)
{}

}

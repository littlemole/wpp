#include "reproweb/web_webserver.h"
#include "reproweb/ctrl/front_controller.h"
#include "reproweb/ctrl/handler.h"
#include "reproweb/json/json.h"
#include "reproweb/view/tpl.h"
#include "reproweb/view/i18n.h"
#include "reproweb/ctrl/ssi.h"
#include "reproweb/ctrl/front_controller.h"
#include "priohttp/http_server.h"
#include "priocpp/api.h"
#include <signal.h>
#include <iostream>

using namespace prio;

namespace reproweb  {

WebServer::WebServer()
    : ctx_(defaultCtx())
{}

WebServer::WebServer(diy::Context& ctx)
    : ctx_(ctx)
{

#ifndef _WIN32        
        signal(SIGPIPE).then( [](int s) {} );
#endif        
        signal(SIGINT).then( [this](int s) 
        { 
            shutdown();
            nextTick([]
            {
                theLoop().exit(); 
            });
        });    
}

WebServer::~WebServer()
{
} 

int WebServer::listen(int port)
{
    int r = 0;
    try
    {
        prio::http_server* server = new prio::http_server;
        
        auto fc = diy::inject<FrontController>(ctx_);
        server->bind(port)
        .then( [fc,port](prio::Request& req, prio::Response& res)
        {
            req.attributes.set("PORT",port);
        	fc->request_handler(req,res);
        })
		.otherwise([](const std::exception& ex)
        {

#ifdef MOL_PROMISE_DEBUG
        	std::cout << "ex:!" << ex.what() << std::endl;
#endif
        });
  
        servers_.push_back(std::unique_ptr<prio::http_server>(server));
    }
    catch( repro::Ex& ex)
    {
        std::cout << "ex: " << ex.msg << std::endl;
        r = 1;
    }  
    return r;
}


int WebServer::listen(prio::SslCtx& ssl, int port)
{
    int r = 0;
    try
    {
        prio::http_server* server = new prio::http_server(ssl);
        
        auto fc = diy::inject<FrontController>(ctx_);
        server->bind(port)
        .then( [fc,port](prio::Request& req, prio::Response& res)
        {
            req.attributes.set("PORT",port);
        	fc->request_handler(req,res);
        })
		.otherwise([](const std::exception& ex)
        {

#ifdef MOL_PROMISE_DEBUG
        	std::cout << "ex:!" << ex.what() << std::endl;
#endif
        });
  
        servers_.push_back(std::unique_ptr<prio::http_server>(server));
    }
    catch( repro::Ex& ex)
    {
        std::cout << "ex: " << ex.msg << std::endl;
        r = 1;
    }  
    return r;
}

void WebServer::listen()
{
    run_config(config_);
}

void WebServer::shutdown()
{
    for( auto& server : servers_)
    {
        server->shutdown();
    }
}


WebServer& WebServer::views(const std::string& path)
{
    view_templates vt(path);

    vt.ctx_register(&ctx_);

    return *this;
}


WebServer& WebServer::htdocs(const std::string& path, const std::string& mime)
{
    static_content sc(path,mime);

    sc.ctx_register(&ctx_);

    return *this;
}

WebServer& WebServer::htdocs(const std::string& path)
{
#ifndef _WIN32
		return htdocs(path, "/etc/mime.types");
#else
		return htdocs(path, "mime.types");
#endif
}


WebServer& WebServer::i18n(const std::string& path,const std::vector<std::string>& locales)
{
    i18n_props ip(path,locales);

    ip.ctx_register(&ctx_);

    return *this;
}


void WebServer::run_config(Json::Value json)
{
    if( json.isMember("i18n"))
    {
        Json::Value localisator = json["i18n"];
        std::string path = localisator["path"].asString();
        Json::Value locale_array = localisator["locales"];

        std::vector<std::string> locales;
        for ( unsigned int i = 0; i < locale_array.size(); i++)
        {
            locales.push_back(locale_array[i].asString());
        }
        
        auto i18n = std::make_shared<I18N>(path,locales);
        ctx_.register_static<I18N>( i18n );
    }

    if( json.isMember("view"))
    {
        std::string path = json["view"].asString();

        auto tpls = std::make_shared<TplStore>();
        tpls->load(path);
        ctx_.register_static<TplStore>( tpls );
    }

    if( json.isMember("htdocs"))
    {
        std::string mime = "mime.types";
        std::string path = json["htdocs"].asString();

        if(json.isMember("mime"))
        {
            mime = json["mime"].asString();
        }

        auto content = std::make_shared<reproweb::StaticContentHandler>(path,mime);
        ctx_.register_static<StaticContentHandler>( content );

        content->register_static_handler(&ctx_);
    }


    if( json.isMember("ssi"))
    {
        Json::Value ssi = json["ssi"];

        std::string path = ssi["htdocs"].asString();
        std::string filter = ssi["filter"].asString();

        http_handler_t handler = [path,filter](prio::Request& req, prio::Response& res)
        {
            res.contentType("text/html");

            std::string tmpl = SSIResolver::tmpl(req,path);

            reproweb::SSIResolver::resolve(req,tmpl)
            .then( [&res](std::string s)
            {
                res.body(s);
                res.ok().flush();
            })
            .otherwise([&res](const std::exception_ptr& ex)
            {
                res.error().flush();
            });
        };

        auto fc = diy::inject<FrontController>(ctx_);
        fc->registerStaticHandler("GET",filter,handler);
    }		

    if(json.isMember("http"))
    {
        Json::Value ports = json["http"]["ports"];
        for( unsigned int i = 0; i < ports.size(); i++)
        {
            int port = ports[i].asInt();
            listen(port);
        }
    }

    if(json.isMember("https"))
    {
        std::string cert = json["https"]["cert"].asString();
        bool enableHttp2 = json["https"]["http2"].asBool();
        Json::Value ports = json["https"]["ports"];

        for( unsigned int i = 0; i < ports.size(); i++)
        {
            Http2SslCtx* sslCtx = new Http2SslCtx();
            sslCtx->load_cert_pem(cert);
            if(enableHttp2)
            {
                sslCtx->enableHttp2();
            }

            sslContexts_.push_back(std::unique_ptr<Http2SslCtx>(sslCtx));
            int port = ports[i].asInt();
            listen(*sslCtx,port);
        }
    }
}

diy::Context& WebServer::defaultCtx()
{
    static WebApplicationContext ctx { diy::inject_modules() };
    return ctx;
}


} // end namespace csgi



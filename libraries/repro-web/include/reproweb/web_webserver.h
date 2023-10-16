#ifndef DEFINE_MOL_HTTP_SERVER_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_DEF_GUARD_DEFINE_

//! \file web_webserver.h
//! \defgroup webserver
//! webserver usage
//! \code{.cpp}
//! WebApplicationContext ctx {
//!   singleton<MyConfig()>()
//! };
//!
//! WebServer webserver(ctx);
//! webserver.configure<MyConfig>();
//! webserver.listen();
//!
//! theLoop().run();
//! \endcode

#include "priohttp/common.h"
#include "priohttp/http_server.h"
#include "diycpp/ctx.h"
#include "reproweb/ctrl/front_controller.h"
#include "reproweb/ctrl/session.h"

namespace reproweb  {

//! \brief HTTP web server
//! \ingroup webserver
//!
class WebServer
{
public:

    //! \private
    WebServer();

    //! construct webserver from DIY context
    WebServer(diy::Context& ctx);

    //! \private
    ~WebServer();    

    //! start webserver listening on given port    
    int listen(int port);
    //! start HTTPS webserver listening on given port    
    int listen(prio::SslCtx& ssl,int port);
    //! listen using port, SSL and other information from config
    void listen();


    //! stop webserver
    void shutdown();

    //! configure WebServer using config value from Config class T
    //!
    //! T must be derived from Config 
    template<class T>
    WebServer& configure()
    {
        auto conf = diy::inject<T>(ctx_);
        config_ = conf->json();
        return *this;
    }

    //! enable sessionfilter for given HTTP method and path and priority
    template<class F = SessionFilter>
    WebServer& session(const std::string& methods, const std::string& paths, int prio)
    {
        session_filter<F> sf(methods,paths,prio);

        sf.ctx_register(&ctx_);

        return *this;
    }

    //! enable sessionfilter as specified in config
    template<class F = SessionFilter>
    WebServer& session()
    {
        std::string methods = config_["session"]["verb"].asString();
        std::string paths = config_["session"]["path"].asString();
        int prio = config_["session"]["prio"].asInt();

        session_filter<F> sf(methods,paths,prio);

        sf.ctx_register(&ctx_);

        return *this;
    }

    //! enable HTTP filter for given method, path and prio with Callback F
    template<class F>
    WebServer& filter(const std::string& methods, const std::string& paths, F f, int prio)
    {
        auto t = ::reproweb::filter(methods,paths,f,prio);

        t.ctx_register(&ctx_);

        return *this;
    }


    //! enable HTTP filter as specified in config
    template<class F>
    WebServer& filter(const std::string& name, F f)
    {
        std::string methods = config_["filter"][name]["verb"].asString();
        std::string paths = config_["filter"][name]["path"].asString();
        int prio = config_["filter"][name]["prio"].asInt();

        auto t = ::reproweb::filter(methods,paths,f,prio);

        t.ctx_register(&ctx_);

        return *this;
    }

    //! set the directory path containing HTML view templates, relative to webserver process binary
    WebServer& views(const std::string& methods);
    //! set the directory path containing HTML static content, relative to webserver process binary
    WebServer& htdocs(const std::string& methods);
    //! set the directory path containing HTML view templates, relative to webserver process binary
    //!
    //! also sets the mime type
    WebServer& htdocs(const std::string& methods, const std::string& mime);
    //! set the directory path containing i18n property files, relative to webserver process binarx    
    WebServer& i18n(const std::string& path,const std::vector<std::string>& locales);

private:

    static diy::Context& defaultCtx();

    Json::Value config_;

    void run_config(Json::Value json);

    diy::Context& ctx_;

    std::vector<std::unique_ptr<prio::http_server>> servers_;
    std::vector<std::unique_ptr<prio::Http2SslCtx>> sslContexts_;
};

}

#endif




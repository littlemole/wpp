#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
#include "reproweb/view/tpl.h"
#include "reprocurl/api.h"

using namespace repro;
using namespace prio;
using namespace reproweb;
using namespace reprocurl;

class Model 
{
public:

    Model()
        :service_endpoint("http://localhost:9876/api/service.json")
    {}

    Future<Json::Value> fetch_greeting()
    {
        auto p = promise<Json::Value>();

        request req(service_endpoint);
        
        fetch(req)
        .then([p](response res)
        {
            p.resolve(JSON::parse(res.content()));
        })
        .otherwise(reject(p));

        return p.future();
    }

private:
    Url service_endpoint;
};


class View 
{
public:
    View()
    {
        templates_.load("/view/");
    }

    void render_greeting(Response& res, Json::Value viewModel)
    {
        res
        .body(templates_.render("index", viewModel ))
        .contentType("text/html")
        .ok()
        .flush();
    }

    void render_error(Response& res, const std::string& e)
    {
        res
        .body(e)
        .contentType("text/html")
        .error()
        .flush();
    }

private:
    reproweb::TplStore templates_;
};

class Controller
{
public:

    Controller(std::shared_ptr<Model> m, std::shared_ptr<View> v)
        : model(m), view(v)
    {}

    void index( Request& req, Response& res)
    {
        model->fetch_greeting()
        .then([this,&res](Json::Value greeting)
        {
            view->render_greeting(res,greeting);
        })
        .otherwise([this,&res](const std::exception& ex)
        {
            view->render_error(res,ex.what());
        });        
    }

private:
    std::shared_ptr<Model> model;
    std::shared_ptr<View> view;
};

int main(int argc, char** argv)
{
    prio::Libraries<prio::EventLoop,reprocurl::ReproCurl> init;

    WebApplicationContext ctx 
    {
        GET( "/", &Controller::index),

        static_content("/htdocs/","/etc/mime.types"),

        diy::singleton<Model()>(),
        diy::singleton<View()>(),
        diy::singleton<Controller(Model,View)>()
    };	

    WebServer server(ctx);
    server.listen(9876);
        
    theLoop().run();

    return 0;
}

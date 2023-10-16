#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
#include "reproweb/view/tpl.h"
#include "diycpp/ctx.h"

using namespace prio;
using namespace reproweb;

class Controller
{
public:
    Controller()
    {
        templates_.load("/view/");
    }

    void index( Request& req, Response& res)
    {
        Json::Value viewModel(Json::objectValue);
        viewModel["greeting"] = "Hello World";
        
        res
        .body(templates_.render("index", viewModel ))
        .contentType("text/html")
        .ok()
        .flush();
    }

private:
    reproweb::TplStore templates_;
};

int main(int argc, char** argv)
{
    prio::Libraries<prio::EventLoop> init;

    WebApplicationContext ctx 
    {
        GET( "/", &Controller::index),

        diy::singleton<Controller()>()
    };	

    WebServer server(ctx);
    server.listen(9876);
        
    theLoop().run();

    return 0;
}

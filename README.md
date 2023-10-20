# Modern C++ for web development. With Promises. And Coroutines.
reactive non-blocking c++ web development with promises. 

# disclaimer

this is a private academic study on how future c++ apis for the web could look like, with an emphasis on promise/coroutine support.

> this software is not considered production ready. use only on your own risk - you have been warned!

for production usage relying on production-ready stacks, have a look at boost::beast.


# motivation
study to explore modern c++'s fitness for modern, server side web development.


# target
from a developer perspective, allow for server side web development at a level of convenience par to Node.js or Java Servlets.

# requirements
- nonblocking io
- single thread model
- no callback hell thanks to promises and/or coroutines
- Dependency Injection
- declarative http routing
- http filters
- websockets and server side events (sse)
- support for JSON/XML
- automagic JSON serialization for REST APIs
- support for HTTP,HTTPS,HTTP2
- support for templating (mustache)
- ssi includes in templates
- perl::mason style autohandler for templates
- i18n support

# c++ dependencies
- [repro](http://github.com/littlemole/repro) exposes the fundamental Promise abstraction to wrap async io
- [cryptoneat](http://github.com/littlemole/cryptoneat) adds support for basic crypto backed by [OpenSSL](https://www.openssl.org/)
- [diy](http://github.com/littlemole/diy) dependency injection for c++
- [patex](https://github.com/littlemole/patex) XML DOM backed by expat
- [metacpp](https://github.com/littlemole/metacpp) for reflection
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) for JSON support
- [mustache](https://github.com/kainjow/Mustache) for templating

# c/c++ basic dependencies

- libevent or boost_asio for main event loop and async IO
- gtest for testing
- jsoncpp for JSON support
- nghttp2 for HTTP/2 support
- openssl for SSL/TLS and crypto
- zlib for compression

# available middleware
non-blocking interconnectivity to resources

- repro-curl for making HTTP client calls serverside
- repro-redis access to Redis key-value store
- repro-mysql access MySQL databases 
- repro-sqlite access SQlite databases

# hello world

main.cpp
```c++
#include "reproweb/ctrl/controller.h"
#include "reproweb/web_webserver.h"
#include "reproweb/view/tpl.h"

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
    init();

    WebApplicationContext ctx 
    {
        GET( "/", &Controller::index),

        singleton<Controller()>()
    };	

    WebServer server(ctx);
    server.listen(9876);
        
    theLoop().run();

    return 0;
}
``` 

index.tpl:
```html
<body>
<head></head>
<body>
<h1>{{greeting}}</h1>
</body>
</html>
```
compile with
```bash
g++ main.cpp -std=c++20 $(pkg-config --libs --cflags reproweb jsoncpp libnghttp2  openssl zlib libevent_pthreads)
``` 
see libraries/repro-web/examples/readme/ for code.

# going async

now extend the example by simulating making an async service call using libcurl. 
actually we will just make an http call to our own webserver that
servers a static json file at /api/service.json.

```c++
...

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
    ...

    WebApplicationContext ctx 
    {
        GET( "/", &Controller::index),

        static_content("/htdocs/","/etc/mime.types"),

        diy::singleton<Model()>(),
        diy::singleton<View()>(),
        diy::singleton<Controller(Model,View)>()
    };

    ...
}

``` 

compile with
```bash
g++ main.cpp -std=c++20 $(pkg-config --libs --cflags reproweb reprocurl jsoncpp libnghttp2  openssl zlib libevent_pthreads libcurl)
``` 

full source in examples/mvc/main.cpp

# coroutines


```c++
...

class Model 
{
public:

    Model()
        :service_endpoint("http://localhost:9876/api/service.json")
    {}

    // co-routine making a non-blocking call using co_await

    Future<Json::Value> fetch_greeting()
    {
        request req(service_endpoint);
        
        response res = co_await fetch(req);

        co_return JSON::parse(res.content());
    }

private:
    Url service_endpoint;
};

class Controller
{
public:

    Controller(std::shared_ptr<Model> m, std::shared_ptr<View> v)
        : model(m), view(v)
    {}

    Async index( Request& req, Response& res)
    {
        try
        {
            Json::Value greeting = co_await model->fetch_greeting();
            view->render_greeting(res,greeting);
        }
        catch(const std::exception& ex)
        {        
            view->render_error(res,ex.what());
        }        
    }

private:
    std::shared_ptr<Model> model;
    std::shared_ptr<View> view;
};

...

``` 

# installation

## docker

assuming a recent installation of docker and docker-compose:

```bash
git clone https://github.com/littlemole/wpp
cd wpp
make image CXX=g++ BACKEND=libevent
``` 

both g++ and clang++ are supported as argument to CXX. Valid values for BACKEND are libevent and boost_asio

g++ and libevent are the default and can be ommited.

at this point do *make run* to start the image with a bash shell and tinker around.

or look at the examples. the examples depend on the image just build. just cd into the example root directory
and invoke ```make up```. smae arguments as for make image above are supported.


```bash
cd examples/hello_world
make up CXX=g++ BACKEND=libevent
``` 
point your browser to https://localhost:9876/ once the container is running.

## linux 

reproduce steps as in [devenv](https://github.com/littlemole/wpp/blob/main/libraries/devenv/Dockerfile) Dockerfile 

this roughly boils down to
- install std c++ dev packages
- install 3dparty C linraries
- fetch and install these static libs from [github.com/littlemole](https://github.com/littlemole)
    - cryptoneat
    - diy
    - patex
    - metacpp
    - repro

use cmake with presets to install. compare the Dockerfile and the helper scripts in docker directory for details.

## windows

windows installation via vcpkg

```bash
git clone https://github.com/littlemole/wpp
cd wpp
win\bootstrap.bat
win\build.bat
```
bootsteap.bat will install a local copy of vcpkg to provide the cmake toolchain file. A dedicated local copy
is used to avoid problems with the visual studio integrated vcpkg, which - at time of writing - does not have
the latest bugfixes.

buid.bat just triggers a debug build using cmake presets.

to play with examples, you do not need to build the library. just run win\bootstrap once as above, then go
to the example folder of interest and run the win\build.bat there.

examples currently supported for windows: hello_world, hello_ws, and hello_ssex.

of the box, but Debug Mode library dependencies will picks some release libs instead the debug ones, so you have to manually fix these if you want to have a simgle config that supports both Release and Debug


## about library and examples debug builds

all libraries and examples do the debug build with -DMOL_PROMISE_DEBUG which conditionally enables
counting of outstanding core entities, useful to track memory leaks during library development.

with -DMOL_PROMISE_DEBUG defined your app has to setup the tracked counters as in [test.h](https://github.com/littlemole/wpp/blob/main/libraries/repro-web/t/test.h). 

LINUX: for normal application development just link to release libs and do a debug build of your app only without defining -DMOL_PROMISE_DEBUG
WIN32: due to the link-time incompat of debug and release builds there is currently no win32 setup to support debug builds without -DMOL_PROMISE_DEBUG. this is a limitation of
the current implementation. 

## IDE support
use an IDE with support form CMake presets. vscode works nicely, make sure you have CMake tools (included by default in recent installations) installed. 

For windows, start vscode from a msvc dev environment. you want to install MS C/C++ extension so you can debug with msvc.
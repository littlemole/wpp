#include "reproweb/ctrl/front_controller.h"
#include "reproweb/ctrl/filter.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include "priohttp/response.h"
#include "priohttp/http_server.h"
#include "priohttp/conversation.h"
#include "reproweb/ctrl/handler_info.h"
#include "reproweb/ctrl/session.h"
#include "diycpp/ctx.h"
#include <fstream>
#include <iostream>


namespace reproweb  {

std::shared_ptr<diy::Context> ctx( prio::Request& req)
 {
     return req.attributes.attr<std::shared_ptr<diy::Context>>("ctx");
 }


////////////////////////////////////////////////////////////////////////////

FrontController::FrontController(std::shared_ptr<diy::Context> ctx)
  : ctx_(ctx) 
{}

void  FrontController::handle_exception(const std::exception& ex, prio::Request& req, prio::Response& res)
{
	for( auto& h : ex_handlers_)
	{
		if ( h->match(ex))
		{
			h->invoke(ex,req,res);
			return;
		}
	}

	for( auto& h : ex_handlers_)
	{
		if ( h->isa(ex))
		{
			h->invoke(ex,req,res);
			return;
		}
	}

    res.error().body(ex.what()).flush();
}


FrontController& FrontController::registerHandler( const std::string& method, const std::string& path, http_handler_t handler)
{
    handlers_.push_back( std::make_unique<HandlerInfo>(method,path,handler) );
    return *this;
}

FrontController& FrontController::registerFilter( const std::string& method, const std::string& path, http_filter_t handler, int prio )
{
    filters_.push_back( std::make_unique<HttpFilterInfo>(method,path,handler,prio) );
    return *this;
}

FrontController& FrontController::registerFlushFilter( const std::string& method, const std::string& path, http_filter_t handler, int prio )
{
    flush_filters_.push_back( std::make_unique<HttpFilterInfo>(method,path,handler,prio) );
    return *this;
}

FrontController& FrontController::registerCompletionFilter( const std::string& method, const std::string& path, http_filter_t handler, int prio )
{
    completion_filters_.push_back( std::make_unique<HttpFilterInfo>(method,path,handler,prio) );
    return *this;
}

FrontController& FrontController::registerStaticHandler( const std::string& method, const std::string& path, http_handler_t handler)
{
    staticHandlers_.push_back( std::make_unique<HandlerInfo>(method,path,handler) );
    return *this;
}

////////////////////////////////////////////////////////////////////////////


template<class T>
HttpFilterInfo* terminator( T t)
{
	return new HttpFilterInfo("","",t);
}


FilterChain::Ptr make_filter_chain(
		std::vector<std::unique_ptr<HttpFilterInfo>>& filters,
		const std::string& method,
		const std::string& path,
		std::function<void(prio::Request&,prio::Response&,std::shared_ptr<FilterChain>)> f
	)
{
    FilterChainBuilder builder;

    for( size_t j = 0; j < filters.size(); j++)
    {
        if ( filters[j]->match(method,path))
        {
            builder.add(filters[j].get());
        }
    }

    FilterChain::Ptr chain = builder.build( terminator(f) );
    return chain;
}

////////////////////////////////////////////////////////////////////////////

void FrontController::handle_request(
		HandlerInfo* h,
	    prio::Request& req,
		prio::Response& res,
		const std::string& method,
		const std::string& path)
{
    FilterChain::Ptr chain =  make_filter_chain(
    	filters_,
		method,
		path,
    	[h] (prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
		{

            h->invoke(req,res);
        }
    );

	auto p = repro::promise<>();

    FilterChain::Ptr flush_chain =  make_filter_chain(
    	flush_filters_,
		method,
		path,
    	[p] (prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
		{
			p.resolve();
		}
    );	

    FilterChain::Ptr completion_chain =  make_filter_chain(
    	completion_filters_,
		method,
		path,
    	[] (prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
		{}
    );

    prio::HttpResponse& response = (prio::HttpResponse&)res;
    response.onFlushHeaders([p,flush_chain,this](prio::Request& req, prio::Response& res)
    {
		prio::nextTick([this,flush_chain,&req,&res]()
		{
			try
			{
				flush_chain->filter(req,res);
			}
			catch(const std::exception& ex)
			{
				handle_exception(ex,req,res);
			}
		});
		return p.future();
    });

    response.onCompletion([completion_chain,this](prio::Request& req, prio::Response& res)
    {
        try
        {
        	completion_chain->filter(req,res);
        }
        catch(const std::exception& ex)
        {
        	handle_exception(ex,req,res);
        }
    });

    try
    {
        chain->filter(req,res);
    }
    catch(const std::exception& ex)
    {
    	handle_exception(ex,req,res);
    }
    return;
}

void FrontController::dispatch(const std::string& path, prio::Request& req, prio::Response& res)
{
	std::string method = req.path.method();

	auto handler = find_handler(path, method, req, res);
	if (!handler)
	{
		res.not_found().flush();
		return;
	}

	try
	{
		handler->invoke(req, res);
	}
	catch (const std::exception& ex)
	{
		handle_exception(ex, req, res);
	}
}

repro::Future<std::string> FrontController::include(const prio::Request& req, const std::string& path)
{
	auto p = repro::promise<std::string>();

	auto subreq = std::make_shared<prio::SubRequest>(); 
	subreq->on(req,path)
	.then( [p]( prio::Request& req, prio::Response& res)
	{
		p.resolve(res.body()); 
	})
	.otherwise(repro::reject(p));

    std::shared_ptr<diy::Context> ctx = std::make_shared<diy::Context>( ctx_.get() );
    subreq->req.attributes.set("ctx", ctx);

	prio::nextTick( [this,subreq,path]() 
	{
		// do not run filter chains again
		//request_handler(subreq->req,subreq->res);

		dispatch(path,subreq->req,subreq->res);
	});

	return p.future();
}

HandlerInfo* FrontController::find_handler(
	const std::string& path, 
	const std::string& method, 
	prio::Request& req, 
	prio::Response& res)
{
	prio::HttpRequest& request = (prio::HttpRequest&)req;

	for (size_t i = 0; i < handlers_.size(); i++)
	{
		auto h = handlers_[i].get();

		if (request.match(h->method(), h->regex(), h->args()))
		{
			return h;
		}
	}

	for (size_t i = 0; i < staticHandlers_.size(); i++)
	{
		auto h = staticHandlers_[i].get();

		prio::patharguments_t args;
		if (h->match(method, path, args))
		{
			return h;
		}
	}

	return nullptr;
}

void FrontController::request_handler( prio::Request& req, prio::Response& res )
{
    std::string method = req.path.method();
    std::string path   = req.path.path();

    std::shared_ptr<diy::Context> ctx = std::make_shared<diy::Context>( ctx_.get() );
    req.attributes.set("ctx", ctx);

	auto handler = find_handler(path, method, req, res);
	if (!handler)
	{
		res.not_found().flush();
		return;
	}

	handle_request(handler, req, res, method, path);
}


////////////////////////////////////////////////////////////////////////////



std::string StaticContentHandler::get_mime( const std::map<std::string,std::string>& mime_, const std::string& fp )
{
	size_t pos = fp.find_last_of('.');
	if ( pos == std::string::npos)
	{
		return "text/plain";
	}
	std::string ext = fp.substr(pos+1);
	if ( mime_.count(ext) == 0 )
	{
		return "text/plain";
	}

	return mime_.at(ext);
}


StaticContentHandler::StaticContentHandler(const std::string& htdocs_path,const std::string& mime_file_path)
	: htdocs_(htdocs_path), mime_(mime_file_path)
{   
} 
   
void StaticContentHandler::register_static_handler(diy::Context* ctx)
{
	std::map<std::string,std::string> map_;
    std::string path_base = prio::get_current_work_dir() + htdocs_;

    std::ifstream ifs;
    ifs.open(mime_);
    while(ifs)
    {
    	std::string line;
    	std::getline(ifs,line);
    	line = prio::trim(line); 
    	if(line.empty()) continue;

    	if(line[0] == '#') continue;

    	size_t pos;
    	pos = line.find_first_of(" \t\r\n");
    	if ( pos != std::string::npos )
    	{
    		std::string mime = line.substr(0,pos);
    		std::string exts = prio::trim(line.substr(pos));
    		auto v = prio::split(exts,' ');

    		for ( auto e : v )
    		{
    			map_[e] = mime;
    		}
    	}
    }
	ifs.close();

    http_handler_t handler = [path_base,map_](prio::Request& req, prio::Response& res)
    {
		std::string path = path_base + req.path.path();
		path = prio::real_path(path);
 
 #ifndef _WIN32
		if ( path.substr(0,path_base.length()) != path_base )
		{
			res.bad_request().flush();
			return;
		} 
#endif

    	res.contentType(StaticContentHandler::get_mime(map_,path));

    	res.header("Access-Control-Allow-Origin", "*");

        prio::task([path]()
		{
        	return prio::slurp(path);
    	})
        .then([&res](std::string data)
		{
        	res.body(data);
        	res.ok().flush();
        });
    };

	auto fc = diy::inject<FrontController>(*ctx);
    fc->registerStaticHandler("GET","/.*",handler);
}

StaticContentHandler::~StaticContentHandler() 
{
}


////////////////////////////////////////////////////////////////////////////



static_content::static_content(const std::string& htdocs_path,const std::string& mime_file_path)
	: htdocs_(htdocs_path), mime_(mime_file_path)
{   
} 
   
void static_content::ctx_register(diy::Context* ctx)
{
	auto content = std::make_shared<StaticContentHandler>(htdocs_,mime_);
	ctx->register_static<StaticContentHandler>(content);
	
	content->register_static_handler(ctx);
}

static_content::~static_content() 
{
}


} // end namespace csgi




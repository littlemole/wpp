#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_PROCESSOR_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_PROCESSOR_DEF_GUARD_

//! \file front_controller.h

#include "reproweb/ctrl/handler_info.h"
#include "diycpp/ctx.h"

namespace reproweb  {

//! grab the diy::Context associated with a HTTP Request 
std::shared_ptr<diy::Context> ctx( prio::Request& req);

//! Front Controller
class FrontController
{
public:

    //! construct FrontController passing diy::Context
    FrontController(std::shared_ptr<diy::Context> ctx);
    
    //! register a HTTP handler
    FrontController& registerHandler( const std::string& method, const std::string& path, http_handler_t handler);
    //! register a HTTP filter
    FrontController& registerFilter( const std::string& method, const std::string& path, http_filter_t filter, int prio = 0);
    //! register a HTTP flush filter
    FrontController& registerFlushFilter( const std::string& method, const std::string& path, http_filter_t filter, int prio = 0);
    //! register a HTTP completion filter
    FrontController& registerCompletionFilter( const std::string& method, const std::string& path, http_filter_t filter, int prio = 0);
    //! register static content handler
    FrontController& registerStaticHandler( const std::string& method, const std::string& path, http_handler_t handler);

    //! register a global exception handler
    template<class E>
    FrontController& registerExceptionHandler( typename ExceptionHandlerInfo<E>::ex_handler_t handler)
    {
    	ex_handlers_.push_back(std::unique_ptr<ExHandlerInfo>(new ExceptionHandlerInfo<E>(handler)));
    	return *this;
    }

    //! \private
    void request_handler( prio::Request& req, prio::Response& res );

    //! dispatch existing request to a different route
	void dispatch(const std::string& path,prio::Request& req, prio::Response& res);

    //! include a HTML fragment
    repro::Future<std::string> include(const prio::Request& req, const std::string& path);


    //! \private
    FrontController(const FrontController& rhs) = delete;
    //! \private
    FrontController& operator=(const FrontController& rhs) = delete;
    
    //! handle an exception via global exception handler
    void handle_exception(const std::exception& ex, prio::Request& req, prio::Response& res);

    //! return a lambda suitable to be passed to otherwise() that passes exceptions to handle_exception method above
    auto handle_ex(prio::Request& req, prio::Response& res)
    {
        return [this,&req,&res]( const std::exception& ex)
        {
            handle_exception(ex,req,res);
        };
    }

private:

    std::vector<std::unique_ptr<HandlerInfo>> handlers_;
    std::vector<std::unique_ptr<HandlerInfo>> staticHandlers_;
    std::vector<std::unique_ptr<HttpFilterInfo>> filters_;
    std::vector<std::unique_ptr<HttpFilterInfo>> flush_filters_;
    std::vector<std::unique_ptr<HttpFilterInfo>> completion_filters_;
    std::vector<std::unique_ptr<ExHandlerInfo>> ex_handlers_;

    void handle_request(
		HandlerInfo* h,
		prio::Request& req,
		prio::Response& res,
		const std::string& method,
		const std::string& path
	);

	HandlerInfo* find_handler(
		const std::string& path,
		const std::string& method, 
		prio::Request& req, 
		prio::Response& res
	);

    std::shared_ptr<diy::Context> ctx_;
};

//! \private
class StaticContentHandler
{
public:

    StaticContentHandler(const std::string& htdocs_path,const std::string& mime_file_path);

    ~StaticContentHandler();

    void register_static_handler(diy::Context* ctx);

private:

    std::string htdocs_;
    std::string mime_;


    static std::string get_mime( const std::map<std::string,std::string>& mime_, const std::string& fp );
};

//! specify static_content when constructing WebApplicationContext
class static_content
{
public:

    typedef static_content type;

    static_content(const std::string& htdocs_path,const std::string& mime_file_path);

    ~static_content();

    void ctx_register(diy::Context* ctx);

private:

    std::string htdocs_;
    std::string mime_;
};


} // end namespace mol
 
#endif


#include <reproweb/ctrl/front_controller.h>
#include "priocpp/api.h"
#include "priohttp/response.h"
#include "priohttp/urlencode.h"
#include "priohttp/http_server.h"
#include <fstream>
#include "reproweb/ctrl/handler_info.h"


namespace reproweb {


////////////////////////////////////////////////////////////////////////////
    
HandlerInfo::HandlerInfo( const std::string& m,  const std::string& p, http_handler_t handler )
{
    method_ = m;
    handler_ = handler;
    
    std::regex r("\\{([^\\}]*)\\}");
    std::smatch match;

    std::string::const_iterator start = p.begin();
    std::string::const_iterator end   = p.end();    
    
    while (std::regex_search (start,end,match,r)) 
    {
        if ( match.size() > 1 )
        {
            args_.push_back(match[1]);
        }
        start = match[0].second;
    }
    std::string replacement("([^/]*)");
    std::string s = std::regex_replace (p,r,replacement);
    path_regex_ = std::regex(s);
    
}

bool HandlerInfo::match(const std::string& m, const std::string& p, prio::patharguments_t& result) 
{
    if ( m != method_ ) 
    {        
        return false;
    }
     
    std::smatch match;
    if ( !std::regex_match(p,match,path_regex_) )
    {
        return false;
    }
        
    prio::patharguments_t args;
    
    for ( size_t i = 0; i < args_.size(); i++)
    {
        args.push_back( prio::pathargument_t(args_[i], prio::Urlencode::decode(match[i+1])) );
    }
    
    result = args;
    return true;
}    

void HandlerInfo::invoke(prio::Request& req, prio::Response& res)
{
	handler_( req, res );
}


////////////////////////////////////////////////////////////////////////////

HttpFilterInfo::HttpFilterInfo( const std::string& m,  const std::string& p, http_filter_t handler, int priority )
{
    method_regex_ = std::regex(m);
    filter_ = handler;
    path_regex_ = std::regex(p);
    priority_ = priority;
}

bool HttpFilterInfo::match(const std::string& m, const std::string& p) 
{
    std::smatch match;
    if ( !std::regex_match(m,match,method_regex_) )
    {        
        return false;
    }
    
    if ( !std::regex_match(p,match,path_regex_) )
    {
        return false;
    }
    return true;
}    


////////////////////////////////////////////////////////////////////////////

  

} // end namespace mol




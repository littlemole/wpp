#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_HANDLER_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_HANDLER_DEF_GUARD_

//! \file handler.h

#include "priohttp/response.h"

//////////////////////////////////////////////////////////////

namespace reproweb  {


//////////////////////////////////////////////////////////////
	
class FrontController;
class FilterChain;

class HandlerInfo;
class HttpFilterInfo;

//! HTTP handler type
//! \ingroup controller
typedef std::function<void(
	prio::Request& req,
	prio::Response& res
)>
http_handler_t;

//! HTTP filter type
//! \ingroup controller
typedef std::function<void(
	prio::Request& req,
	prio::Response& res,
	std::shared_ptr<FilterChain> chain
)>
http_filter_t;



} // end namespace mol

#endif


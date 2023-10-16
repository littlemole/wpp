#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_CURL_COMMON_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_CURL_COMMON_DEF_GUARD_


#include "priocpp/common.h"
#include <vector>

//////////////////////////////////////////////////////////////


namespace reprocurl {

namespace impl {

	typedef int socket_t;

	typedef void CURL;
	typedef void CURLM;
	struct curl_slist;
}



typedef std::pair<std::string,std::string> header_t;
typedef std::vector<header_t> headers_t;

typedef std::pair<std::string,std::string> pathargument_t;
typedef std::vector<pathargument_t> patharguments_t;


} // close namespaces

#endif


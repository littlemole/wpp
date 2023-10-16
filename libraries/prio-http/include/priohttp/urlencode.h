#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_URLENCODE_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_URLENCODE_DEF_GUARD_

//! \file urlencode.h
#include "priohttp/common.h"

namespace prio  {

//! \brief static Urlencode helpers
class Urlencode
{
public:

    //! url-decode given string
    static std::string decode(const std::string& s);
    //! url-decode given c-style string
    static std::string decode(const char* s, int len);
    //! url-encode given string
    static std::string encode(const std::string& s);
    //! url-encode given c-style string
    static std::string encode(const char* s, int len);
};

} // close namespaces

#endif


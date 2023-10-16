#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_PARAMS_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_PARAMS_DEF_GUARD_

//! \file queryparams.h
#include "priohttp/common.h"
#include <set>

namespace prio  {

//! \brief Query Parameter class
class QueryParams
{
public:

    //! construct empty query params set
    QueryParams();
    //! parse querystring into QueryParams object
    QueryParams(const std::string& s);

    //! check whether query parameter for key exists
    bool exists(const std::string& key) const;
    //! get query parameter for given key
    std::string get(const std::string& key) const;
    //! get all the query parameter keys
    std::set<std::string> keys() const;
    //! get all query params with given name (for arrays)
    std::vector<std::string> array(const std::string& key) const;

    //! remove a given query params
    void remove(const std::string& key);

    //! set specific parameter. existing ones with same name will be overwritten.
    void set(const std::string& key, const std::string& value);
    //! add a specific parameter without overwriting existing ones
    void add(const std::string& key, const std::string& value);
    //! helper to return query parameter for given parameter
    std::string& operator[] (const std::string& key);
    //! return a string representation suitable for HTTP transmission
    std::string toString() const;

protected:
    std::vector<std::pair<std::string,std::string>> params_;
};

} // close namespaces

#endif


#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_ARG_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_ARG_DEF_GUARD_

#include "priohttp/common.h"

//! \file arg.h
 
namespace prio  {


typedef std::pair<std::string,std::string> pathargument_t;
typedef std::vector<pathargument_t> patharguments_t;

/**
 * \brief HTTP Path Args
 */
class Args
{
public:

    //! \private
    Args( const patharguments_t& args);

    //! check wether a named path argument exists
    bool exists(const std::string& key) const;
    //! get single value for named path argument
    std::string get(const std::string& key) const;
    //! get vector of strings holding the available path keys
    std::vector<std::string> keys() const;
    
private:
    patharguments_t args_; 
};


} // close namespaces

#endif


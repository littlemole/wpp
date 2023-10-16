#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_ICONV_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_ICONV_DEF_GUARD_

//! \file ic.h

#include <string>
#include <vector>
#include <iconv.h>

namespace reproweb  {

//! iconv helper
class miconv
{
public:

    //! construct miconv for from and to charsets
    miconv( const char* from, const char* to);
    ~miconv();

    //! convert string from input to output charset
    std::string convert( const std::string& input );
    
    //! return available encodings
    static std::vector<std::string> encodings();

private:

    iconv_t ict_;
};

} // end namespace mol

#endif



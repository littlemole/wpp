#include "reproweb/tools/ic.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "priocpp/common.h"
#include "reprocpp/ex.h"

namespace reproweb  {



miconv::miconv( const char* from, const char* to)
{
    ict_ = iconv_open( to, from );
    if ( (size_t)ict_ == (size_t)-1 ) {
        throw repro::Ex("encoding not supported ");
    }
}

miconv::~miconv()
{
    iconv_close(ict_);
}

std::string miconv::convert( const std::string& input )
{
    size_t bufsize = 4 * input.size();
    std::vector<char> outbuf(bufsize,0);

    size_t is = input.size();
    size_t os = bufsize;
    char* pIn = (char*)input.c_str();
    char* pOut = &(outbuf[0]);

    iconv(ict_,NULL,NULL,&pOut,&os);

    while( 0 < is ) {
        size_t s = iconv( ict_, &pIn, &is, &pOut, &os );
        if ( s == (size_t)-1 ) {
            if ( errno == E2BIG ) {
            }
            else if ( true ) {
                ++pIn;
                --is;
            }
        }
    }

    return std::string (&outbuf[0],(pOut-&outbuf[0]));
}

std::vector<std::string> miconv::encodings()
{
    std::vector<std::string> result;

#ifndef _WIN32

    std::ostringstream oss;
    FILE* file = popen("iconv -l", "r");
    ssize_t read = 0;
    char * line = 0;
    size_t len = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        std::string s(line,read);
        result.push_back( prio::trim(s).substr(0,s.size()-3) );
        free(line);
        line=0;
    }
    if(line)
    {
        free(line);
    }

    pclose(file);
#endif

    return result;
}


} // end namespace mol



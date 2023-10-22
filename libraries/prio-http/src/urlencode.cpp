#include "priohttp/urlencode.h"
#include "cryptoneat/common.h"

using namespace cryptoneat;

namespace prio  {


// URLencode helpers
inline char c2x(const char &x) {         return x > 9 ? x + 55: x + 48; }

inline char x2c ( unsigned char* in)
{
	char digit;
	digit = ( (*in) >= 'A' ? (((*in) & 0xdf) - 'A')+10 : ((*in) - '0'));
	digit *= 16;
	in++;
	digit += ((*in) >= 'A' ? (((*in) & 0xdf) - 'A')+10 : ((*in) - '0'));
	return(digit);
}

std::string Urlencode::decode(const std::string& s)
{
	return decode(s.c_str(),s.size());
}

std::string Urlencode::encode(const std::string& s)
{
	return encode(s.c_str(),s.size());
}


std::string Urlencode::encode(const char* in, size_t n)
{
    std::string out("");
    const size_t nLen = n + 1;

    unsigned char* pInBuf  =(unsigned char*)in;
    unsigned char* pOutTmp = NULL;
    unsigned char* pInTmp  = NULL;

    uchar_buf buf(nLen  * 3 - 2);

    pInTmp  = pInBuf;
    pOutTmp = &buf;
    while (*pInTmp)
    {
        unsigned char c = *((unsigned char*)pInTmp);
        int i = 0;
        i += c;
        if( (isalnum(i)))
        *pOutTmp++ = *pInTmp;
        else
        if( isspace(i) && ((i!='\n') && (i!='\r')) )
			*pOutTmp++ = '+';
        else
        {
            *pOutTmp++ = '%';
            *pOutTmp++ = c2x(*pInTmp>>4);
            *pOutTmp++ = c2x(*pInTmp%16);
        }
        pInTmp++;
    }
    *pOutTmp = 0;
	out = buf.toString(pOutTmp-&buf);
    return out;
}

std::string Urlencode::decode( const char* in, size_t n )
{
	size_t len = n;
	unsigned char* inBuff = (unsigned char*)(in);

	uchar_buf buf(len+1);
	unsigned char* outBuff = &buf;

    while( *inBuff )
    {
	    if ( *inBuff == '%' )
        {
            inBuff++;
			*outBuff = x2c(inBuff);
            inBuff++;
        }
        else if ( *inBuff == '+' )
        {
            *outBuff = ' ';
        }
        else
        {
			*outBuff=*inBuff;
        }
        outBuff++;
        inBuff++;
    }
    *outBuff =0;
	return buf.toString(outBuff-&buf);
}


} // close namespaces

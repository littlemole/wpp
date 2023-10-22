#include "reprocurl/urlencode.h"
#include <curl/curl.h>


namespace reprocurl   	{


std::string Urlencode::decode(const std::string& s)
{
    return decode( s.c_str(), s.size() );
}

std::string Urlencode::decode(const char* s, size_t len)
{
	std::ostringstream oss;
	for( int i = 0; i < len; i++) {
		if ( s[i] == '+' ) {
			oss << " ";
		}
		else {
			oss << s[i];
		}
	}
	std::string tmp = oss.str();

    char* c = curl_unescape( tmp.c_str() , (int) tmp.size() );
    std::string result(c);
    curl_free(c);
    return result;
}

std::string Urlencode::encode(const std::string& s)
{
    return encode( s.c_str(), s.size() );
}

std::string Urlencode::encode(const char* s, size_t len)
{
    CURL* curl = curl_easy_init();
    char* c = curl_easy_escape( curl , s , (int)len );
    std::string result(c);
    curl_free(c);
    curl_easy_cleanup(curl);
    return result;
}

} // close namespaces


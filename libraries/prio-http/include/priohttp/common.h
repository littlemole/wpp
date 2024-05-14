#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_COMMON_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_COMMON_DEF_GUARD_

//! \file common.h

#include "reprocpp/promise.h"
#include "priocpp/api.h"

#include <vector>
#include <map>

//////////////////////////////////////////////////////////////

namespace prio  {

std::string unquote(const std::string& str );
std::string unescape_html( const std::string& str );
std::string escape_html(const std::string& in );

std::string get_executable_dir();
void set_current_work_dir(const std::string& path);
std::string get_current_work_dir();

std::vector<std::string> split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> split(const std::string &s, std::string delim);
std::vector<std::string> glob (const std::string& f);
bool is_directory(const std::string& path);
//////////////////////////////////////////////////////////////

std::string real_path( const std::string& str );
std::string safe_path( const std::string& path );
std::string slurp( const std::string& fp );

//////////////////////////////////////////////////////////////

#define MOL_CPP_PASTE2(x,y) x##y
#define MOL_CPP_PASTE(x,y) MOL_CPP_PASTE2(x,y)
#define MOL_CPP_IDENTIFIER(x) MOL_CPP_PASTE(x,__LINE__)

//////////////////////////////////////////////////////////////

struct Http2SslCtxImpl;

//! Http2 aware SSL context
class Http2SslCtx : public prio::SslCtx
{
public:
	Http2SslCtx();
	~Http2SslCtx();

	//! load server TLS certs from file
    void load_cert_pem(const std::string& file);
	//! enable listening to http2 requests
	void enableHttp2();

	//! \private
	void enableHttp2Client();
};


} // end namespaces

#endif


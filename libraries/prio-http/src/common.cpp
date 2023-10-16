#define _LIBCPP_ENABLE_CXX20_REMOVED_TYPE_TRAITS 1

#include <sstream>
#include <fstream>

#ifndef _WIN32
#include <dirent.h>

#else
#include "priohttp/msdirent.h"
#include <stdlib.h>  
#define PATH_MAX MAX_PATH
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex>


#include <limits.h>
#include <stdlib.h>

#ifdef _WIN32
#include <inttypes.h>
typedef int ssize_t;
#endif

#include <openssl/ssl.h>
#include <nghttp2/nghttp2.h>
#include "priocpp/impl/asio.h"
#include "priocpp/impl/event.h"
#include "priohttp/common.h"


#ifdef _WIN32
#include <direct.h>
#include <pathcch.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#endif    


namespace prio  {

std::string unquote(const std::string& str )
{
	if(str.size() > 1 && str[0] == '"'  && str[str.size()-1] == '"' ) return str.substr(1,str.size()-2);
	if(str.size() > 1 && str[0] == '\'' && str[str.size()-1] == '\'') return str.substr(1,str.size()-2);
    return str;
}    

std::string real_path( const std::string& path )
{
#ifdef _WIN32
    char buf[MAX_PATH];
    GetFullPathNameA(path.c_str(), MAX_PATH, buf, NULL);

    char canonical[MAX_PATH];
    PathCanonicalizeA(canonical,buf);

    std::string result(canonical);
    return result;
#else
    char buf[PATH_MAX];
    if(!realpath(path.c_str(), buf))
    {
        throw repro::Ex("real path not avail");
    }

    std::string result(buf);
    return result;
#endif   
}

std::string get_executable_dir()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    GetModuleFileName( NULL, buf, MAX_PATH);
    std::string result(buf);
    return result;
#else
    char buf[PATH_MAX];
    if( readlink("/proc/self/exe", buf, PATH_MAX) == -1 )
    {
        throw repro::Ex("readlink failed");
    }
    std::string result(buf);
    size_t pos = result.find_last_of("/");
    if(pos != std::string::npos)
    {
        return result.substr(0,pos);
    }
    return result;
#endif    
}

void set_current_work_dir(const std::string& path)
{
#ifdef _WIN32
    _chdir(path.c_str());
#else
    if( chdir(path.c_str()) == -1 )
    {
        throw repro::Ex("chdir failed");
    };
#endif    
}


std::string get_current_work_dir()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    _getcwd(buf,MAX_PATH);

    std::string result(buf);
    return result;
#else
    char buf[PATH_MAX];
    if(getcwd(buf,PATH_MAX) == NULL)
    {
        throw repro::Ex("getcwd failed");
    }

    std::string result(buf);
    return result;    
#endif    
}

std::string escape_html(const std::string& in )
{
    std::ostringstream out;
    size_t p = 0;
    size_t len = in.size();
    while( ( p < len ) )
    {
        switch ( in[p] )
        {
            case '&' :
            {
                out << "&amp;";
                break;
            }
            case '<' :
            {
                out << "&lt;";
                break;
            }
            case '>' :
            {
                out << "&gt;";
                break;
            }
            default :
            {
                out << in[p];
                break;
            }
        }
        p++;
    }
    return out.str();
}

std::string unescape_html( const std::string& str )
{
    std::ostringstream out;
    size_t len = str.size();
    for ( size_t i = 0; i < len; i++ )
    {
        if ( str[i] == '&' )
        {
			if ( str.substr(i,4) == "&lt;" )
            {
				out << "<";
                i+=3;
            }
            else
			if ( str.substr(i,4) == "&gt;" )
            {
                out << ">";
                i+=3;
            }
            else
	        if ( str.substr(i,5) == "&amp;" )
            {
                out << "&";
                i+=4;
            }
		}
        else
        {
			out << str[i];
        }
    }
    return out.str();
}


std::vector<std::string> split(const std::string &s, char delim, std::vector<std::string> &elems) 
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if(!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) 
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


std::vector<std::string> split(const std::string &s, std::string delim)
{
    std::vector<std::string> elems;

    size_t last = 0;
    size_t pos = s.find(delim,last);
    while(pos != std::string::npos)
    {
    	if(pos != 0)
    	{
    		std::string tmp = s.substr(last,pos-last);

    		if(!tmp.empty())
    		{
    			elems.push_back(tmp);
    		}
    	}
    	last = pos + delim.size();
    	pos = s.find(delim,last);
    }

    if ( last < s.size())
    {
    	elems.push_back(s.substr(last));
    }
    return elems;
}

std::vector<std::string> glob(const std::string& f)
{
  DIR           *d;
  struct dirent *dir;
  
  std::vector<std::string> result;
  
  d = opendir(f.c_str());
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG)
        {   
            std::string n = std::string(dir->d_name);
            if ( n != "." && n != "..")
            {
                result.push_back(dir->d_name);
            }
        }
    }
    closedir(d);
  }
  return result;
}

std::string safe_path( const std::string& path )
{
    std::regex e ("\\.\\.");
    return std::regex_replace(path,e,"");
}


std::string slurp( const std::string& fp )
{
    std::ostringstream oss;
    std::ifstream ifs;
    ifs.open(fp,std::ios::binary);
    if(!ifs)
    {
        return "";
    }

    while(ifs)
    {
        char buf[4096];
        ifs.read(buf,4096);
        std::streamsize s = ifs.gcount();
        oss.write(buf,s);
    }

    ifs.close();
    return oss.str();
}

unsigned char next_proto_list[256];
size_t next_proto_list_len;

int next_proto_cb(SSL *ssl, const unsigned char **data,unsigned int *len, void *arg) 
{
  *data = next_proto_list;
  *len = (unsigned int)next_proto_list_len;
  return SSL_TLSEXT_ERR_OK;
}

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
          unsigned char *outlen, const unsigned char *in,
		  unsigned int inlen, void *arg) 
{
  int rv = nghttp2_select_next_protocol((unsigned char **)out, outlen, in, inlen);

  if (rv != 1) {
    return SSL_TLSEXT_ERR_NOACK;
  }

  return SSL_TLSEXT_ERR_OK;
}
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L



#ifdef PROMISE_USE_LIBEVENT

struct Http2SslCtxImpl : public SslCtxImpl
{
	Http2SslCtxImpl();
	~Http2SslCtxImpl();
	
	void enableHttp2();
	void enableHttp2Client();
};



Http2SslCtxImpl::Http2SslCtxImpl()
{

}

Http2SslCtxImpl::~Http2SslCtxImpl()
{
}


void Http2SslCtxImpl::enableHttp2(  )
{
	next_proto_list[0] = NGHTTP2_PROTO_VERSION_ID_LEN;
	memcpy(&next_proto_list[1], NGHTTP2_PROTO_VERSION_ID,
	NGHTTP2_PROTO_VERSION_ID_LEN);
	next_proto_list_len = 1 + NGHTTP2_PROTO_VERSION_ID_LEN;
	
	SSL_CTX_set_next_protos_advertised_cb(ctx, next_proto_cb, NULL);
	
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	SSL_CTX_set_alpn_select_cb(ctx, alpn_select_proto_cb, NULL);
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L
	  
}

static int select_next_proto_cb(SSL *ssl, unsigned char **out,
	unsigned char *outlen, const unsigned char *in,
	unsigned int inlen, void *arg) 
{
	if (nghttp2_select_next_protocol(out, outlen, in, inlen) <= 0) 
	{
		std::cout << "err select_next_proto_cb ACK failed -no http2" << std::endl;
		return SSL_TLSEXT_ERR_NOACK;
	}
	return SSL_TLSEXT_ERR_OK;
}

void Http2SslCtxImpl::enableHttp2Client(  )
{
	next_proto_list[0] = NGHTTP2_PROTO_VERSION_ID_LEN;
	memcpy(&next_proto_list[1], NGHTTP2_PROTO_VERSION_ID,
	NGHTTP2_PROTO_VERSION_ID_LEN);
	next_proto_list_len = 1 + NGHTTP2_PROTO_VERSION_ID_LEN;

	SSL_CTX_set_next_proto_select_cb(ctx, select_next_proto_cb, NULL);
	
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	  SSL_CTX_set_alpn_protos(ctx, (const unsigned char *)"\x02h2", 3);
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L	  
}



Http2SslCtx::Http2SslCtx()
{
    ctx.reset(new Http2SslCtxImpl());
}

Http2SslCtx::~Http2SslCtx()
{
}


void Http2SslCtx::load_cert_pem(const std::string& file)
{
	ctx->loadKeys(file);
}


void Http2SslCtx::enableHttp2()
{
	((Http2SslCtxImpl*)(ctx.get()))->enableHttp2();
}


void Http2SslCtx::enableHttp2Client()
{
	((Http2SslCtxImpl*)(ctx.get()))->enableHttp2Client();
}


#endif // USE_LIBEVENT


#ifdef PROMISE_USE_BOOST_ASIO

struct Http2SslCtxImpl : public SslCtxImpl
{
	Http2SslCtxImpl();
	~Http2SslCtxImpl();

    	
	void enableHttp2();
	void enableHttp2Client();
};



Http2SslCtxImpl::Http2SslCtxImpl()
{
	ssl.set_options(
        	boost::asio::ssl::context::default_workarounds
	      | boost::asio::ssl::context::no_sslv2
        );
}

Http2SslCtxImpl::~Http2SslCtxImpl()
{
}


void Http2SslCtxImpl::enableHttp2(  )
{
    SSL_CTX* sslCtx = ssl.native_handle();

    next_proto_list[0] = NGHTTP2_PROTO_VERSION_ID_LEN;
    memcpy(&next_proto_list[1], NGHTTP2_PROTO_VERSION_ID,
    NGHTTP2_PROTO_VERSION_ID_LEN);
    next_proto_list_len = 1 + NGHTTP2_PROTO_VERSION_ID_LEN;
    
    SSL_CTX_set_next_protos_advertised_cb(sslCtx, next_proto_cb, NULL);
        
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        SSL_CTX_set_alpn_select_cb(sslCtx, alpn_select_proto_cb, NULL);
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L

}

static int select_next_proto_cb(SSL *ssl, unsigned char **out,
	unsigned char *outlen, const unsigned char *in,
	unsigned int inlen, void *arg) 
{
	if (nghttp2_select_next_protocol(out, outlen, in, inlen) <= 0) 
	{
		std::cout << "err select_next_proto_cb ACK failed -no http2" << std::endl;
		return SSL_TLSEXT_ERR_NOACK;
	}
	return SSL_TLSEXT_ERR_OK;
}

void Http2SslCtxImpl::enableHttp2Client(  )
{
    SSL_CTX* sslCtx = ssl.native_handle();
    next_proto_list[0] = NGHTTP2_PROTO_VERSION_ID_LEN;
    memcpy(&next_proto_list[1], NGHTTP2_PROTO_VERSION_ID,
    NGHTTP2_PROTO_VERSION_ID_LEN);
    next_proto_list_len = 1 + NGHTTP2_PROTO_VERSION_ID_LEN;

    SSL_CTX_set_next_proto_select_cb(sslCtx, select_next_proto_cb, NULL);
        
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
          SSL_CTX_set_alpn_protos(sslCtx, (const unsigned char *)"\x02h2", 3);
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L 
}



Http2SslCtx::Http2SslCtx()
{
    ctx.reset(new Http2SslCtxImpl());
}

Http2SslCtx::~Http2SslCtx()
{
}


void Http2SslCtx::load_cert_pem(const std::string& file)
{
	SslCtx::load_cert_pem(file);
}


void Http2SslCtx::enableHttp2()
{
	((Http2SslCtxImpl*)(ctx.get()))->enableHttp2();
}


void Http2SslCtx::enableHttp2Client()
{
	((Http2SslCtxImpl*)(ctx.get()))->enableHttp2Client();
}


#endif // USE_BOOST_ASIO


} // end namespaces



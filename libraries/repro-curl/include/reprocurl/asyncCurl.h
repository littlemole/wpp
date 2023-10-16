#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CLIENT_CURL_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CLIENT_CURL_DEF_GUARD_

#include "reprocurl/common.h"
#include "priocpp/api.h"
 
 
//////////////////////////////////////////////////////////////


namespace reprocurl   	{


std::string& ca_path();

class CurlEasy;

class CurlMulti
{
public:

	CurlMulti();
    ~CurlMulti();

    void remove(reprocurl::impl::CURL* c);
    bool add(reprocurl::impl::CURL* c);
    void perform();

    void dispose();

private:

    void init();

    bool check_multi_info();
    bool check_err_code(const char *where, int code);

    void onTimer();
    bool onSocket(reprocurl::impl::socket_t s, short kind, CurlEasy* easy);

    int on_sock_cb(reprocurl::impl::CURL *e, reprocurl::impl::socket_t s, int what, CurlEasy *cbp);
    int on_multi_timer_cb(reprocurl::impl::CURLM *multi, long timeout_ms);

    static int sock_cb(reprocurl::impl::CURL *e, reprocurl::impl::socket_t s, int what, void *cbp, void *sockp);
    static int multi_timer_cb(reprocurl::impl::CURLM *multi, long timeout_ms,void* cbp);

    reprocurl::impl::CURLM* multi_;
    int still_running_;

    std::unique_ptr<prio::Timeout> timeout_;
};

CurlMulti& curl_multi();

class CurlEasy : public std::enable_shared_from_this<CurlEasy>
{
public:


#ifdef PROMISE_USE_LIBEVENT
    prio::IO io_read;
    prio::IO io_write;
#endif    

	typedef std::shared_ptr<CurlEasy> Ptr;
	typedef repro::Promise<Ptr> PromiseType;
	typedef repro::Future<Ptr> FutureType;

	static Ptr create();

    ~CurlEasy();

	void dispose();

    Ptr url(const std::string& url);
    Ptr data( const std::string& formdata );
    Ptr method( const std::string& m );
    Ptr header(const std::string& key, const std::string& val);
    Ptr verbose();
    Ptr insecure();
    Ptr ca_path(const std::string& ca);
	Ptr ca_info(const std::string& ca);
	Ptr client_cert(const std::string& ca);

    repro::Future<Ptr> perform();

    long status();
    std::string response_body();

    headers_t& response_headers();
    std::string response_header( const std::string& key );

    void reject(const std::exception& ex)
    {
    	promise_.reject(ex);
    	dispose();
    }

    void resolve()
    {
    	promise_.resolve(shared_from_this());
    	dispose();
    }

    bool pending_;
    int what_;

private:

    CurlEasy();
    CurlEasy(const CurlEasy&) = delete;
    CurlEasy& operator=(const CurlEasy&) = delete;

    void init();
    void init_request();
    void reset();


    int on_write_cb(void *data, size_t size, size_t nmemb);
    size_t on_rcvHeaders_cb(void *buffer, size_t size, size_t nmemb);

    static int write_cb(void *data, size_t size, size_t nmemb, void* userp);
    static size_t rcvHeaders_cb(void *buffer, size_t size, size_t nmemb, void *userp);

    std::string url_;
    std::string method_;
    std::string formdata_;

    PromiseType promise_;

    reprocurl::impl::CURL* easy_;
    reprocurl::impl::curl_slist* headers_;

    std::ostringstream oss_;
    headers_t response_headers_;

    Ptr self_;
};

CurlEasy::Ptr async_curl();


class ReproCurl
{
public:
	ReproCurl();
};


} // close namespaces

#endif


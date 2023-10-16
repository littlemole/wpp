#ifdef PROMISE_USE_LIBEVENT

#include <curl/curl.h>
#include <reprocurl/asyncCurl.h>
#include <iostream>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

using namespace repro;
using namespace prio;

namespace reprocurl   	{

std::string& ca_path()
{
	static std::string ca;
	return ca;
}


CurlMulti& curl_multi()
{
	static CurlMulti cm;
	return cm;
}


CurlEasy::Ptr async_curl()
{
	return CurlEasy::create();
}

CurlMulti::CurlMulti()
	: 
	  multi_(nullptr),
	  still_running_(1),
	  timeout_(new Timeout)
{
	init();
}


CurlMulti::~CurlMulti()
{
	if(timeout_)
	{
		timeout_->cancel();	
	}

	curl_multi_cleanup(multi_);
}

void CurlMulti::dispose()
{
	if(timeout_)
	{
		timeout_->cancel();
		timeout_.reset();
	}
}

///////////////////////////////////////////////////////////////


void CurlMulti::remove(impl::CURL* c)
{
	curl_multi_remove_handle(multi_,c);
}

bool CurlMulti::add(impl::CURL* c)
{
	CURLMcode rc = curl_multi_add_handle(multi_, c);
	if(! check_err_code("new_conn: curl_multi_add_handle", (int)rc))
	{
		std::cerr << "CurlMulti::add failed" << std::endl;
		return false;
	}
	return true;
}

void CurlMulti::perform()
{
	curl_multi_socket_action((void*)multi_, CURL_SOCKET_TIMEOUT, 0, &still_running_);
}

void CurlMulti::init()
{
	multi_ = curl_multi_init();

	curl_multi_setopt(multi_, CURLMOPT_SOCKETFUNCTION, &sock_cb);
	curl_multi_setopt(multi_, CURLMOPT_SOCKETDATA, this);
	curl_multi_setopt(multi_, CURLMOPT_TIMERFUNCTION, &multi_timer_cb);
	curl_multi_setopt(multi_, CURLMOPT_TIMERDATA, this);
}



///////////////////////////////////////////////////////////////

// handle results

bool CurlMulti::check_multi_info()
{
	CURL* easy    = 0;
	CURLMsg* msg  = 0;
	CURLcode r;
	int msgs_left = 0;

	while ((msg = curl_multi_info_read(multi_, &msgs_left)))
	{
		if (msg->msg == CURLMSG_DONE)
		{
			easy = msg->easy_handle;
			r = msg->data.result;
		}
	}

	if(easy) // we are done
	{
		CurlEasy* ceasy = nullptr;
		curl_easy_getinfo(easy,CURLINFO_PRIVATE,(char**)&ceasy);
		if ( r != CURLE_OK )
		{
			repro::Ex ex("async curl ex");
			ceasy->reject(ex);
			return true;
		}
		else
		{
			ceasy->resolve();
			return true;
		}
	}

	if(!still_running_)
	{
		timeout_->cancel();
		return true;
	}
	return false;
}

// check for error codes

bool CurlMulti::check_err_code(const char *where, int code)
{
	if ( CURLM_OK != code )
	{
		const char *s;
		switch (code)

		{
			case     CURLM_BAD_HANDLE:         s="CURLM_BAD_HANDLE";         break;
			case     CURLM_BAD_EASY_HANDLE:    s="CURLM_BAD_EASY_HANDLE";    break;
			case     CURLM_OUT_OF_MEMORY:      s="CURLM_OUT_OF_MEMORY";      break;
			case     CURLM_INTERNAL_ERROR:     s="CURLM_INTERNAL_ERROR";     break;
			case     CURLM_UNKNOWN_OPTION:     s="CURLM_UNKNOWN_OPTION";     break;
			case     CURLM_LAST:               s="CURLM_LAST";               break;
			case     CURLM_BAD_SOCKET:         s="CURLM_BAD_SOCKET";
				std::cerr << where << " " << s << std::endl;
				return true;

			default: s="CURLM_unknown";
			break;
		}

		std::ostringstream oss;
		oss << "async curl ex " << where << " " << s << std::endl;
		std::cerr << oss.str();

		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////

// callbacks

bool CurlMulti::onSocket(impl::socket_t fd, short kind, CurlEasy* easy)
{
	int action =
		(kind & CURL_POLL_IN ? CURL_CSELECT_IN : 0) |
		(kind & CURL_POLL_OUT ? CURL_CSELECT_OUT : 0);

	CURLMcode rc = curl_multi_socket_action(multi_, fd, action, &still_running_);
	if(!check_err_code("event_cb: curl_multi_socket_action", (int) rc))
	{
		repro::Ex ex("curl error socket io");
		easy->reject(ex);
		return true;
	}
	return check_multi_info();
}

void CurlMulti::onTimer()
{
	CURLMcode rc = curl_multi_socket_action(multi_,CURL_SOCKET_TIMEOUT, 0, &still_running_);
	if(!check_err_code("timer_cb: curl_multi_socket_action", (int) rc))
	{
		std::cout << "failed on timer!" << std::endl;
		return;
	}
	check_multi_info();
}


int CurlMulti::on_sock_cb(CURL *curl, impl::socket_t sock, int what, CurlEasy* easy)
{
	easy->what_ = what;

	if(timeout_)
		timeout_->cancel();

	if (what & CURL_POLL_REMOVE)
	{
		return 0;
	}

	if(easy->pending_)
	{
		return 0;
	}

	if (what & CURL_POLL_IN)
	{
		easy->io_write.cancel();
		easy->io_read.cancel();
		easy->io_read.onRead(sock)
		.then( [this,curl,sock,what,easy]()
		{	
			if(onSocket(sock,what,easy))
			{
				return;
			}
			on_sock_cb(curl,sock,easy->what_,easy);
		})
		.otherwise( [](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});
	}
	else
	if (what & CURL_POLL_OUT)
	{			
		easy->io_read.cancel();
		easy->io_write.cancel();
		easy->io_write.onWrite(sock)
		.then( [this,curl,sock,what,easy]()
		{	
			if(onSocket(sock,what,easy))
			{
				return;
			}

			on_sock_cb(curl,sock,easy->what_,easy);
		})
		.otherwise( [](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});
	}

	return 0;
}

int CurlMulti::on_multi_timer_cb(CURLM *multi, long timeout_ms)
{
	if(!timeout_)
		return 0;

	timeout_->cancel();
	if(timeout_ms == -1)
	{
		return 0;
	}

	timeout_->after(timeout_ms)
	.then( [this]()
	{
		onTimer();
	});

	return 0;
}


// socketdata on multi, value from multi_assign is sockp
int CurlMulti::sock_cb(CURL *e, impl::socket_t s, int what, void *cbp, void *sockp)
{
	CurlMulti *curl = (CurlMulti*) cbp;

	CurlEasy* ceasy = nullptr;
	curl_easy_getinfo(e,CURLINFO_PRIVATE,(char**)&ceasy);
	if(!ceasy)
	{
		return 0;
	}

	return curl->on_sock_cb(e,s,what,ceasy);
}

// socketdata
int CurlMulti::multi_timer_cb(CURLM *multi, long timeout_ms,void* cbp)
{
	CurlMulti *curl = (CurlMulti*) cbp;

	if(!curl)
		return 0;

	return curl->on_multi_timer_cb(multi,timeout_ms);
}


// #################################################


CurlEasy::CurlEasy()
	:
	  pending_(false),
	  what_(0),
	  easy_(nullptr),
	  headers_(NULL)
{
	REPRO_MONITOR_INCR(curl);
}


CurlEasy::~CurlEasy()
{
	if(easy_)
	{
		curl_easy_cleanup(easy_);
		easy_ = nullptr;
	}

    if(headers_ != NULL)
    {
        curl_slist_free_all((::curl_slist*)headers_ );
        headers_ =NULL;
    }	
	REPRO_MONITOR_DECR(curl);
}

CurlEasy::Ptr CurlEasy::create()
{
	Ptr ptr = std::shared_ptr<CurlEasy>(new CurlEasy);
	ptr->promise_ = repro::promise<Ptr>();
	ptr->init();
	ptr->self_ = ptr;
	return ptr;
}


void CurlEasy::dispose()
{
	curl_multi().remove(easy_); 
	curl_easy_setopt(easy_,CURLOPT_PRIVATE,nullptr);
	self_.reset();
}

CurlEasy::Ptr CurlEasy::insecure()
{
	 curl_easy_setopt(easy_, CURLOPT_SSL_VERIFYPEER, false);
	 curl_easy_setopt(easy_, CURLOPT_SSL_VERIFYHOST, false);
	 return shared_from_this();
}

CurlEasy::Ptr CurlEasy::ca_path(const std::string& ca)
{
	 curl_easy_setopt(easy_, CURLOPT_CAPATH, ca.c_str() );
	 return shared_from_this();
}

CurlEasy::Ptr CurlEasy::ca_info(const std::string& ca)
{
	curl_easy_setopt(easy_, CURLOPT_CAINFO, ca.c_str());
	return shared_from_this();
}


CurlEasy::Ptr CurlEasy::client_cert(const std::string& cert)
{
	curl_easy_setopt(easy_, CURLOPT_SSLCERT, cert.c_str());
	return shared_from_this();
}

Future<CurlEasy::Ptr> CurlEasy::perform()
{
	auto ptr = shared_from_this();
	nextTick( [ptr]()
	{
		ptr->init_request();
	});
	
	return promise_.future();
}


///////////////////////////////////////////////////////////////

// setters

CurlEasy::Ptr CurlEasy::url(const std::string& url)
{
	url_ = url;
	return shared_from_this();
}

CurlEasy::Ptr CurlEasy::data( const std::string& formdata )
{
	formdata_ = formdata;
	return shared_from_this();
}

CurlEasy::Ptr CurlEasy::method( const std::string& m )
{
    method_ = m;
	return shared_from_this();
}

CurlEasy::Ptr CurlEasy::header(const std::string& key, const std::string& val)
{
    std::ostringstream oss;
    oss << key << ":" << val;
    headers_ = (impl::curl_slist*)curl_slist_append((::curl_slist*)headers_, oss.str().c_str() );
	return shared_from_this();
}

CurlEasy::Ptr CurlEasy::verbose()
{
    curl_easy_setopt(easy_, CURLOPT_VERBOSE, 1L);
	return shared_from_this();
}

///////////////////////////////////////////////////////////////

// result getters

long CurlEasy::status()
{
    long http_code = 0;
    curl_easy_getinfo (easy_, CURLINFO_RESPONSE_CODE, &http_code);
    return http_code;
}

std::string CurlEasy::response_body()
{
	return oss_.str();
}

headers_t& CurlEasy::response_headers()
{
    return response_headers_;
}

std::string CurlEasy::response_header( const std::string& key )
{
    for ( size_t i = 0; i < response_headers().size(); i++ )
    {
        if ( strcasecmp( response_headers()[i].first.c_str(), key.c_str()) == 0 )
        {
            return response_headers()[i].second;
        }
    }
    return "";
}

///////////////////////////////////////////////////////////////

// start of a request


void CurlEasy::init()
{
	easy_ =  curl_easy_init();
	curl_easy_setopt(easy_,CURLOPT_PRIVATE,this);
	if( !reprocurl::ca_path().empty() )
	{
		curl_easy_setopt(easy_, CURLOPT_CAPATH, reprocurl::ca_path().c_str() );
	}
}

void CurlEasy::reset()
{
	if(easy_ != NULL)
	{
		curl_multi().remove(easy_);
	}
	curl_easy_reset(easy_);
	curl_easy_setopt(easy_,CURLOPT_PRIVATE,this);
	response_headers_.clear();
	oss_.str("");
}

void CurlEasy::init_request()
{
	curl_easy_setopt(easy_, CURLOPT_NOSIGNAL,1L);
	curl_easy_setopt(easy_, CURLOPT_HTTPHEADER, headers_);
    curl_easy_setopt(easy_, CURLOPT_HEADERFUNCTION, &rcvHeaders_cb);
    curl_easy_setopt(easy_, CURLOPT_WRITEHEADER, this);

	curl_easy_setopt(easy_, CURLOPT_URL, url_.c_str());
	curl_easy_setopt(easy_, CURLOPT_WRITEFUNCTION, &write_cb);
	curl_easy_setopt(easy_, CURLOPT_WRITEDATA, this);
//	curl_easy_setopt(easy_, CURLOPT_VERBOSE, 1L);

	if ( !formdata_.empty() )
	{
		curl_easy_setopt(easy_, CURLOPT_POSTFIELDS, formdata_.c_str() );
		curl_easy_setopt(easy_, CURLOPT_POSTFIELDSIZE, formdata_.size() );
	}

	if( !method_.empty() )
	{
		curl_easy_setopt(easy_, CURLOPT_CUSTOMREQUEST, method_.c_str() );
	}

	if(!curl_multi().add(easy_))
	{
		// handle err
		promise_.reject(repro::Ex("add multi failed"));
	}
}

///////////////////////////////////////////////////////////////

// handle results


///////////////////////////////////////////////////////////////

// callbacks



int CurlEasy::on_write_cb(void *data, size_t size, size_t nmemb)
{
	size_t len = size * nmemb;
	if ( len > 0)
	{
		std::string s((char*)data,len);
		oss_ << s;
	}
    return len;
}


size_t CurlEasy::on_rcvHeaders_cb(void *buffer, size_t size, size_t nmemb)
{
    size_t len = size * nmemb;
    if ( len > 0)
    {
		std::string s = std::string( (char*)buffer, len );
		size_t pos = s.find(':');
		if ( pos != std::string::npos )
		{
			std::string key = trim(s.substr(0,pos));
			std::string val = trim(s.substr(pos+1,s.size()-pos-3));
			response_headers_.push_back( header_t(key,val) );
		}
    }
    return len;
}


// WRITEDATA on easy
int CurlEasy::write_cb(void *data, size_t size, size_t nmemb, void* userp)
{
	CurlEasy* curl = (CurlEasy*)(userp);
	return curl->on_write_cb(data,size,nmemb);
}


// CURLOPT_WRITEHEADER , should be HEADERDATA
size_t CurlEasy::rcvHeaders_cb(void *buffer, size_t size, size_t nmemb, void *userp)
{
	CurlEasy* curl = (CurlEasy*)(userp);
	return curl->on_rcvHeaders_cb(buffer,size,nmemb);
}


} // close namespaces


#endif

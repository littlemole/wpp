#ifdef PROMISE_USE_LIBEVENT

#include <priocpp/api.h>
#include <priocpp/task.h>
#include <priocpp/impl/event.h>
#include "priocpp/ssl_connection.h"

#include <fcntl.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>


#include <iostream>

#ifdef _WIN32
#include <inttypes.h>
typedef int ssize_t;
#define close_socket ::closesocket
#define SHUT_WR SD_SEND 
#else
#define close_socket ::close
#endif

using namespace repro;

namespace prio      {


extern unsigned char next_proto_list[256];
extern size_t next_proto_list_len;
	
// forward
int check_err_code(SSL* ssl, int len, int want);


SslConnectionImpl::SslConnectionImpl()
	: fd(-1),
	  ssl(0)
{
}

SslConnectionImpl::~SslConnectionImpl()
{
}


SslConnection::SslConnection(SslConnectionImpl* impl)
	: impl_(impl)
{
	timeouts_ = connection_timeouts();
	REPRO_MONITOR_INCR(SslConnection);
}


SslConnection::~SslConnection()
{
	close();
	REPRO_MONITOR_DECR(SslConnection);
}

connection_timeout_t& SslConnection::timeouts()
{
	return timeouts_;
}


void SslConnection::ssl_do_connect(Promise<Connection::Ptr> p,SslCtx& ctx)
{
	int r = SSL_connect(impl_->ssl);
	int s = check_err_code(impl_->ssl,r,EV_READ);
	if ( s == 0)
	{
		if(ctx.verify_certs())
		{
			/* Step 1: verify a server certificate was presented during the negotiation */
			X509* cert = SSL_get_peer_certificate(impl_->ssl);
			if(cert) { X509_free(cert); } /* Free immediately */
			if(NULL == cert) 
			{
				p.reject(repro::Ex("no SSL cert received from server!"));
				return;
			}

			/* Step 2: verify the result of chain verification */
			/* Verification performed according to RFC 4158    */
			long res = SSL_get_verify_result(impl_->ssl);
			if(!(X509_V_OK == res)) 
			{
				p.reject(repro::Ex("SSL cert rchain validation failed!"));
				return;
			}
		}
		p.resolve(Ptr(this));
		return;
	}
	if ( s < 0 )
	{
		p.reject(Ex());
		delete this;
		return;
	}

	impl_->e_ = onEvent( impl_->fd, s)
	->callback( [this,p,&ctx](socket_t /* fd */, short what)
	{
		if(what == EV_TIMEOUT)
		{
			p.reject(IoTimeout("IO timeout in SslConnection::do_connect"));
			delete this;
			return;
		}
		ssl_do_connect(p,ctx);
	})
	->add(timeouts().connect_timeout_s,0);
}

Future<Connection::Ptr> SslConnection::connect(const std::string& host, int port,  SslCtx& ctx )
{
	auto p = promise<Connection::Ptr>();

	auto impl = new SslConnectionImpl;
	auto ptr = new SslConnection(impl);		

	std::string h = host;

	dnsResolver()
	.connect(h,port)
	.then( [p,host,impl,ptr,&ctx](socket_t fd)
	{
		impl->fd = fd;

		SSL* ssl = SSL_new(ctx.ctx->ctx);
		impl->ssl = ssl;

		SSL_set_fd(ssl,(int)fd);
		SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE|SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

		SSL_set_tlsext_host_name(ssl, host.c_str());

		if(ctx.verify_certs())
		{
			SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
			if (!SSL_set1_host(ssl, host.c_str())) 
			{
				throw repro::Ex("could not set hostname verification");
			}
			SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);
		}
		ptr->ssl_do_connect(p,ctx);
	})
	.otherwise(reject(p));

	return p.future();	
}


void SslConnection::do_ssl_read(Promise<Connection::Ptr,std::string> p, short what)
{
	impl_->e_ = onEvent(impl_->fd,what)
	->callback( [this,p,what](socket_t /* fd */, short /* w */) 
	{
		if( what & EV_TIMEOUT)
		{
			p.reject( IoTimeout("timeout in do_ssl_read") );
			return;
		}

		char c[1024];
		int n = 1024;
 
		int len = SSL_read( impl_->ssl, c, n );
		if ( len > 0 )
		{
			p.resolve(shared_from_this(),std::string(c,len));
			return;
		}

		int s = check_err_code(impl_->ssl,len,EV_READ);
		if ( s <= 0 )
		{
			p.reject(Ex("ssl io EOF"));
			return;
		}
		do_ssl_read(p,s);
	});

	if (timeouts().rw_timeout_s != 0)
	{
		impl_->e_->add(timeouts().rw_timeout_s, 0);
	}
	else
	{
		impl_->e_->add();
	}
}


Future<Connection::Ptr, std::string> SslConnection::read()
{
	auto p = promise<Connection::Ptr,std::string>();

	char c[1024];
	int n = 1024;

	int len = SSL_read( impl_->ssl, c, n );
	if ( len > 0 )
	{
		std::string data(c,len);

		auto ptr = shared_from_this();
		nextTick( [p,data,ptr]()
		{
			p.resolve(ptr,data);
		});
		return p.future();
	}

	int s = check_err_code(impl_->ssl,len,EV_READ);
	if ( s <= 0 )
	{
		nextTick( [p]()
		{
			p.reject(Ex("ssl io EOF read"));
		});
		return p.future();
	}

	do_ssl_read(p,s);

	return p.future();
}


void SslConnection::do_ssl_read( Promise<Connection::Ptr,std::string> p, short what, std::shared_ptr<std::string> buffer, std::shared_ptr<size_t> want)
{
	impl_->e_ = onEvent(impl_->fd,what)
	->callback( [this,p,want,buffer](socket_t /* fd */, short /* w */ )
	{
		while(true)
		{
			char c[1024];
			size_t n = *want;

			int len = SSL_read( impl_->ssl, c, (int) n );
			if ( len > 0 )
			{
				buffer->append(std::string(c,len));

				if ( *want - len <= 0)
				{
					p.resolve(shared_from_this(),*buffer);
					return;
				}
				*want -= len;
				continue;
			}

			int s = check_err_code(impl_->ssl,len,EV_READ);
			if ( s <= 0 )
			{
				p.reject(Ex("ssl io EOF"));
				return;
			}

			do_ssl_read(p,s,buffer,want);
			return;
		}
	});

	if (timeouts().rw_timeout_s != 0)
	{
		impl_->e_->add(timeouts().rw_timeout_s, 0);
	}
	else
	{
		impl_->e_->add();
	}
}

Future<Connection::Ptr, std::string> SslConnection::read(size_t s)
{
	auto p = promise<Connection::Ptr,std::string>();

	std::shared_ptr<size_t> want = std::make_shared<size_t>(s);
	std::shared_ptr<std::string> buffer = std::make_shared<std::string>();

	std::vector<char> c(s,0);
	size_t n = s;

	while(true)
	{
		int len = SSL_read( impl_->ssl, &c[0], (int) n );
		if ( len > 0 )
		{
			std::string tmp(&c[0],len);
			buffer->append(tmp);

			if ( *want -len <= 0 )
			{
				auto ptr = shared_from_this();
				nextTick( [p,buffer,ptr]()
				{
					p.resolve(ptr,*buffer);
				});
				return p.future();
			}
			*want -= len;
			continue;
		}

		int s = check_err_code(impl_->ssl,len,EV_READ);
		if ( s <= 0 )
		{
			nextTick( [p]()
			{
				p.reject(Ex("ssl io EOF"));
			});
			return p.future();
		}

		do_ssl_read(p,s,buffer,want);
		return p.future();
	}

	return p.future();
}

void SslConnection::do_ssl_write(Promise<Connection::Ptr> p, std::string data, std::shared_ptr<size_t> written, short what)
{
	impl_->e_ = onEvent(impl_->fd,what)
	->callback([this,p,data,written](socket_t /* fd */, short /* w */)
	{
		while(true)
		{
			int len = SSL_write( impl_->ssl, data.c_str() + *written, (long)( data.size() - *written) );
			if ( len > 0 )
			{
				*written = *written + len;
				if (*written >= data.size())
				{
					p.resolve(shared_from_this());
					return;
				}
				continue;
			}

			int s = check_err_code(impl_->ssl,len,EV_WRITE);
			if ( s <= 0 )
			{
				p.reject(Ex("ZERO RETURN"));
				return;
			}

			do_ssl_write(p,data,written,s);
			return;
		}
	});

	if (timeouts().rw_timeout_s != 0)
	{
		impl_->e_->add(timeouts().rw_timeout_s, 0);
	}
	else
	{
		impl_->e_->add();
	}
}

Future<Connection::Ptr> SslConnection::write( const std::string& data)
{
	auto p = promise<Connection::Ptr>();

	auto written = std::make_shared<size_t>(0);

	while(true)
	{
		int len = SSL_write( impl_->ssl, data.c_str() + *written, (long)(data.size() - *written) );
		if ( len > 0 )
		{
			*written = *written + len;
			if (*written >= data.size())
			{
				auto ptr = shared_from_this();
				nextTick( [p,ptr]()
				{
					p.resolve(ptr);
				});
				return p.future();
			}
			continue;
		}

		int s = check_err_code(impl_->ssl,len,EV_WRITE);
		if ( s <= 0 )
		{
			nextTick( [p]()
			{
				p.reject(Ex("ssl io ex"));
			});
			return p.future();
		}

		do_ssl_write(p,data,written,s);
		return p.future();
	}

	return p.future();
}


void SslConnection::close()
{
	cancel();
	if(impl_->fd != -1)
	{
		close_socket(impl_->fd);
		impl_->fd = -1;
	}
	if(impl_->ssl)
	{
		SSL_free(impl_->ssl);
		impl_->ssl = 0;
	}
}



Future<> SslConnection::shutdown()
{
	auto p = promise<>();

	if(impl_->fd == -1)
		return p.resolved();

	int s = SSL_get_shutdown(impl_->ssl);

	int r = 0;
    if ( s == SSL_RECEIVED_SHUTDOWN )
	{
		r = SSL_shutdown(impl_->ssl);
		close();
		return p.resolved();
	}
	else
	{
		r = SSL_shutdown(impl_->ssl);
		if ( r == 0 )
		{
			::shutdown(impl_->fd,SHUT_WR);
			r = SSL_shutdown(impl_->ssl) == 1;
			close();
		}
		return p.resolved();
	}

	return p.future();
}

void SslConnection::cancel()
{
	if ( impl_ && impl_->e_ )
	{
		impl_->e_->cancel();
	}
}

bool SslConnection::isHttp2Requested()
{
	return impl_->isHttp2Requested();
}

std::string SslConnection::common_name()
{
	return impl_->common_name();
}

std::string SslConnectionImpl::common_name()
{
	X509* cert = SSL_get_peer_certificate(ssl);

	if(cert) 
	{ 
		char buf[1025];

		X509_NAME_get_text_by_NID(X509_get_subject_name(cert),NID_commonName, buf, 1024);

		std::string res = buf;
		X509_free(cert); 
		return res;
	}
	return ""; 
}

bool SslConnectionImpl::isHttp2Requested()
{
	const unsigned char *alpn = NULL;
    unsigned int alpnlen = 0;
	
	SSL_get0_next_proto_negotiated(ssl, &alpn, &alpnlen);

#if OPENSSL_VERSION_NUMBER >= 0x10002000L

	if (alpn == NULL) 
	{
		SSL_get0_alpn_selected(ssl, &alpn, &alpnlen);
	}

#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L
	
	if (alpn == NULL || alpnlen != 2 || memcmp("h2", alpn, 2) != 0) 
	{
		return false;
	}
	return true;
}

int check_err_code(SSL* ssl, int len, int want)
{
	int r = SSL_get_error(ssl,len);
	switch(r)
	{
		case SSL_ERROR_SYSCALL :
		{
			if( would_block())
			{
				return want;
			}

			return -1;
		}
		case SSL_ERROR_ZERO_RETURN :
		{
			return -1;
		}
		case SSL_ERROR_NONE:
		{
			return 0;
		}
		case SSL_ERROR_WANT_READ:
		{
			return EV_READ;
		}
		case SSL_ERROR_WANT_WRITE:
		{
			return EV_WRITE;
		}
		default :
		{

			std::cout << "SSL SOME ERR " << ERR_error_string(ERR_get_error(), NULL) <<  " " << r << std::endl;
		}
	}
	return -1;
}




SslCtxImpl::SslCtxImpl()
	: verify_(true), verify_client_(false),ca_path_("")
{
	ctx = SSL_CTX_new(TLS_method());
	if (!ctx) 
	{
	  throw(Ex("Could not create SSL/TLS context"));
	}
	SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

	SSL_CTX_set_default_verify_paths(ctx);
}

SslCtxImpl::~SslCtxImpl()
{
	SSL_CTX_free(ctx);
}

void SslCtxImpl::verify_certs(bool v)
{
	verify_ = v;
}

bool SslCtxImpl::verify_certs()
{
	return verify_;
}

bool SslCtxImpl::verify_client()
{
	return verify_client_;
}

void SslCtxImpl::set_ca_path(const std::string& ca)
{
	ca_path_ = ca;
	if(ca.empty())
	{
		SSL_CTX_set_default_verify_paths(ctx);
	}
	else
	{
		int r = SSL_CTX_load_verify_locations(ctx,ca_path_.c_str(),NULL);
		std::cout << "SSL_CTX_load_verify_locations returned: " << r << std::endl;
	}
}

std::string SslCtxImpl::get_ca_path()
{
	return ca_path_;
}

void SslCtxImpl::set_client_ca(const std::string& pem)
{
	STACK_OF(X509_NAME) *cert_names;

	cert_names = SSL_load_client_CA_file(pem.c_str());
	if (cert_names == NULL)
	{
		std::cout << "load client ca file failed!" << std::endl;
		exit(1);
	}

	SSL_CTX_set_client_CA_list(ctx, cert_names);

	verify_client_ = true;
}



int next_proto_cb(SSL *ssl, const unsigned char **data,unsigned int *len, void *arg);

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
          unsigned char *outlen, const unsigned char *in,
		  unsigned int inlen, void *arg);
#endif

void SslCtxImpl::loadKeys( const std::string& keyfile )
{
	std::cout << "Load cer " << keyfile << std::endl;

	if(!(SSL_CTX_use_certificate_file(ctx,	keyfile.c_str(),SSL_FILETYPE_PEM)))
	{
		auto e = ERR_get_error();
		auto s = ERR_error_string(e,0);
		std::cout << e << ":" << s << std::endl;
		throw Ex("Can't read certificate file");
	}

	if(!(SSL_CTX_use_PrivateKey_file(ctx,keyfile.c_str(),SSL_FILETYPE_PEM)))
		throw Ex("Can't read key file");

	SSL_CTX_set_options(
		ctx,
		SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
	  	SSL_OP_NO_COMPRESSION |
	  	SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
	);
/*  
    EC_KEY *ecdh;
	ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (!ecdh) 
	{
		throw(Ex("EC_KEY curvename failed "));
	}
	SSL_CTX_set_tmp_ecdh(ctx, ecdh);
	EC_KEY_free(ecdh);	
*/	  
	int r = SSL_CTX_set_cipher_list(ctx,"ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK");

	std::cout << "set cipher: " << r << std::endl;
   
}


SslCtx::SslCtx()
	: ctx(new SslCtxImpl)
{

}

SslCtx::~SslCtx()
{
}


void SslCtx::load_cert_pem(const std::string& file)
{
	ctx->loadKeys(file);
}

void SslCtx::verify_certs(bool v)
{
	ctx->verify_certs(v);
}

bool SslCtx::verify_certs()
{
	return ctx->verify_certs();
}

bool SslCtx::verify_client()
{
	return ctx->verify_client();
}

void SslCtx::set_ca_path(const std::string& ca)
{
	ctx->set_ca_path(ca);
}

std::string SslCtx::get_ca_path()
{
	return ctx->get_ca_path();	
}

void SslCtx::set_client_ca(const std::string& pem)
{
	ctx->set_client_ca(pem);
}


} // close namespaces



#endif

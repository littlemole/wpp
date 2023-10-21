#ifdef PROMISE_USE_BOOST_ASIO

#include "priocpp/impl/asio.h"
#include "priocpp/api.h"
#include "priocpp/ssl_connection.h"

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

SslConnectionImpl::SslConnectionImpl(SslCtx& ssl)
	: socket(asioLoop().io(), ssl.ctx->ssl), 
	  resolver(asioLoop().io()),
	  ctx(ssl.ctx->ssl)
{
}

bool SslConnectionImpl::isHttp2Requested()
{
	const unsigned char *alpn = NULL;
	unsigned int alpnlen = 0;

	SSL* ssl = socket.native_handle();
	
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


std::string SslConnectionImpl::common_name()
{
	X509* cert = SSL_get_peer_certificate(socket.native_handle());

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


SslConnection::SslConnection(SslConnectionImpl* impl)
	: timeouts_(connection_timeouts()),
	  impl_(impl)
{
	REPRO_MONITOR_INCR(SslConnection);
}


SslConnection::~SslConnection()
{
	cancel();
	close();
	REPRO_MONITOR_DECR(SslConnection);	
}

connection_timeout_t& SslConnection::timeouts()
{
	return timeouts_;
}

Future<Connection::Ptr> SslConnection::connect(const std::string& host, int port,  SslCtx& ctx )
{
	auto p = promise<Connection::Ptr>();

	auto impl = new SslConnectionImpl(ctx);
	auto c = new SslConnection(impl);		

	boost::asio::ip::tcp::resolver::query query(host, "");
	impl->resolver.async_resolve(
		query,
		[host,&ctx,impl,p,port,c](const boost::system::error_code& /*error*/, boost::asio::ip::tcp::resolver::iterator iterator)
		{
			while(iterator != boost::asio::ip::tcp::resolver::iterator())
			{
				boost::asio::ip::tcp::endpoint end = *iterator;
				if (end.protocol() != boost::asio::ip::tcp::v4())
				{
					iterator++;
					continue;
				}

				boost::asio::ip::tcp::endpoint endpoint(
					end.address(), 
					port
				);
		
				SSL_set_tlsext_host_name(impl->socket.native_handle(), host.c_str());

				impl->socket
				.lowest_layer()
				.async_connect( 
					endpoint, 
					[host,impl,p,c](const boost::system::error_code& error)
					{
						if(error)
						{
							delete c;
							p.reject(Ex("connection failed"));
						}
						else
						{
							impl->socket.set_verify_mode(boost::asio::ssl::verify_peer|boost::asio::ssl::verify_fail_if_no_peer_cert);
							impl->socket.set_verify_callback(boost::asio::ssl::rfc2818_verification(host));
							impl->socket.async_handshake(
								boost::asio::ssl::stream_base::client,
								[host,impl,p,c](const boost::system::error_code& error)
								{
									if(error)
									{
										delete c;
										p.reject(Ex("ssl handshake client connect failed"));
										return;		
									}
#if BOOST_VERSION < 106599
									boost::asio::ip::tcp::socket::non_blocking_io non_blocking_io(true);
									impl->socket.lowest_layer().io_control(non_blocking_io);
#else
									impl->socket.lowest_layer().non_blocking(true);
#endif									
									p.resolve(Connection::Ptr( c ));
								}
							);
						}
					}
				);
				return;
			}
			
			delete c;
			p.reject(Ex("connect dns lookup failed"));
		}
	);

	return p.future();
}


Future<Connection::Ptr, std::string> SslConnection::read()
{
	auto p = promise<Connection::Ptr,std::string>();
	auto ptr = shared_from_this();

	if (timeouts_.rw_timeout_s != 0)
	{
		impl_->timer.after(timeouts_.rw_timeout_s).then(cancellation(p, impl_->socket.lowest_layer()));
	}

	impl_->socket.async_read_some(
		boost::asio::buffer(impl_->data,impl_->max_length),
		[this,ptr,p](const boost::system::error_code& error,std::size_t bytes_transferred)
		{
			impl_->timer.cancel();

			if(error)
			{
				if (is_io_cancelled(error)) return;

				p.reject(Ex(std::string("read failed ")+error.message()));
			}
			else
			{
				p.resolve(ptr,std::string(impl_->data,bytes_transferred));
			}
		}
	);

	return p.future();
}

Future<Connection::Ptr, std::string> SslConnection::read(size_t s)
{
	auto p = promise<Connection::Ptr,std::string>();
	auto ptr = shared_from_this();
	
	if (timeouts_.rw_timeout_s != 0)
	{
		impl_->timer.after(timeouts_.rw_timeout_s).then(cancellation(p, impl_->socket.lowest_layer()));
	}

	std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>(s,0);

	async_read(
		impl_->socket,
		boost::asio::buffer(&(buffer->at(0)),s),
		[this,ptr,p,buffer](const boost::system::error_code& error,std::size_t bytes_transferred)
		{
			impl_->timer.cancel();

			if(error)
			{
				if (is_io_cancelled(error)) return;
				
				p.reject(Ex(std::string("read() failed")+error.message()));
			}
			else
			{
				p.resolve( ptr, std::string( &(buffer->at(0)), bytes_transferred) );
			}
		}
	);

	return p.future();
}


Future<Connection::Ptr> SslConnection::write( const std::string& data)
{
	auto p = promise<Connection::Ptr>();
	auto ptr = shared_from_this();

	if (timeouts_.rw_timeout_s != 0)
	{
		impl_->timer.after(timeouts_.rw_timeout_s).then(cancellation(p, impl_->socket.lowest_layer()));
	}

	std::shared_ptr<std::string> buffer = std::make_shared<std::string>(data);

	async_write(
		impl_->socket,
		boost::asio::buffer(buffer->data(),buffer->size()),
		[this,p,ptr,buffer](const boost::system::error_code& error,std::size_t /*bytes_transferred*/ )
		{
			impl_->timer.cancel();

			if(error)
			{
				if (is_io_cancelled(error)) return;
				
				p.reject(Ex(std::string("write failed")+error.message()));
			}
			else
			{
				p.resolve( ptr );
			}
		}
	);

	return p.future();
}


void SslConnection::close()
{
	if(impl_->socket.lowest_layer().is_open())
	{
		impl_->socket.lowest_layer().close();
	}	
}

Future<> SslConnection::shutdown()
{
	auto p = promise<>();
	
	if(!impl_->socket.lowest_layer().is_open())
		return p.resolved();

	impl_->socket.shutdown();

	read()
	.then([this,p](Connection::Ptr,std::string /*data*/ )
	{
		close();
		p.resolve();
	})
	.otherwise([this,p](const std::exception& /*ex*/ )
	{
		close();
		p.resolve();
	});

	return p.future();
}

void SslConnection::cancel()
{
	try
	{
		impl_->timer.cancel();

		if(impl_->socket.lowest_layer().is_open())
		{
			impl_->socket.lowest_layer().cancel();
		}
	}
	catch(const std::exception& ex)
	{
		std::cout << "ex in cancel(): " << ex.what() << std::endl;
	}
	catch(...)
	{
		std::cout << "some ex in cancel(): ?"  << std::endl;
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


SslCtxImpl::SslCtxImpl()
	  : ssl( boost::asio::ssl::context::tlsv12 ), verify_(true),verify_client_(false),ca_path_("")
{
	ssl.set_default_verify_paths();
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

	if(verify_)
	{
		ssl.set_verify_mode(boost::asio::ssl::verify_peer);
	}

	if(ca.empty())
	{
		ssl.set_default_verify_paths();
	}
	else
	{
		ssl.load_verify_file(ca_path_);
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

	SSL_CTX_set_client_CA_list(ssl.native_handle(), cert_names);

	verify_client_ = true;
}

SslCtx::SslCtx()
	: ctx(new SslCtxImpl)
{
	SSL_CTX* sslCtx = ctx->ssl.native_handle();
	SSL_CTX_set_options(
		sslCtx,
		SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
	  	SSL_OP_NO_COMPRESSION |
	  	SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
	);
}

SslCtx::~SslCtx()
{}

void SslCtx::load_cert_pem(const std::string& file)
{
	std::cout << "load cer " << file << std::endl;
	ctx->ssl.use_certificate_chain_file(file);
	ctx->ssl.use_private_key_file(file, boost::asio::ssl::context::pem);

	SSL_CTX* sslCtx = ctx->ssl.native_handle();
/*
	EC_KEY *ecdh = 0;
	ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (!ecdh) 
	{
		throw(Ex("EC_KEY curvename failed "));
	}
	SSL_CTX_set_tmp_ecdh(sslCtx, ecdh);
	EC_KEY_free(ecdh);	
*/
	int r = SSL_CTX_set_cipher_list(sslCtx,"ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK");

	std::cout << "set cipher: " << r << std::endl;

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



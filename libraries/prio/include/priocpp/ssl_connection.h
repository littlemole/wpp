#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CONNECT_SSL_HANDLER_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CONNECT_SSL_HANDLER_DEF_GUARD_

#include "priocpp/api.h"



namespace prio      {


class SslConnectionImpl;

class SslConnection : public Connection
{
public:

	typedef std::shared_ptr<Connection> Ptr;

	SslConnection(SslConnectionImpl* impl);
	virtual ~SslConnection();

	static repro::Future<Connection::Ptr> connect(const std::string& host, int port, SslCtx& ctx);

	virtual repro::Future<Connection::Ptr, std::string> read();
	virtual repro::Future<Connection::Ptr, std::string> read(size_t n);
	virtual repro::Future<Connection::Ptr> write(const std::string& data);

	virtual void close();
	virtual repro::Future<> shutdown();
	virtual void cancel();

	virtual connection_timeout_t& timeouts();

	virtual bool isHttp2Requested();
	virtual std::string common_name();
	
protected:

	SslConnection(SslConnection&) = delete;
	SslConnection& operator=(SslConnection&) = delete;

	void ssl_do_connect(repro::Promise<Connection::Ptr> p, SslCtx& ctx);
	void do_ssl_read(repro::Promise<Connection::Ptr,std::string> p, short what);
	void do_ssl_read(repro::Promise<Connection::Ptr,std::string> p, short what, std::shared_ptr<std::string> buffer, std::shared_ptr<size_t> want);
	void do_ssl_write( repro::Promise<Connection::Ptr> p, std::string data, std::shared_ptr<size_t> written, short what);

	connection_timeout_t timeouts_;

	std::unique_ptr<SslConnectionImpl> impl_;

};


} // close namespaces

#endif


#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_API_HANDLER_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_API_HANDLER_DEF_GUARD_


#include "priocpp/common.h"
#include "priocpp/loop.h"
#include "priocpp/timeout.h"
#include "priocpp/signal.h"
#include "priocpp/url.h"
#include "priocpp/callback.h"
#include <tuple>


/**
 * \file api.h
 */

namespace prio      {

enum ASYNC_WHAT {

	ASYNC_TIMEOUT = 0x01,
	ASYNC_READ = 0x02,
	ASYNC_WRITE	= 0x04,
	ASYNC_SIGNAL = 0x08,
	ASYNC_PERSIST =	0x10	
};

/**
*  \brief call std::function<void()> f on the eventloop asynchronously.
*  \param f completion handler
*
*  This function will invoke the callback asybchronously on the main event loop.
*/
void nextTick(const std::function<void()> f) noexcept;
//void nextTick(std::function<void()>&& f) noexcept;


class EventLoop 
{
public:
	EventLoop();
};

/**
* \class Libraries
* \brief initialize requires libraries
* \param Args... Library RAII loader classes to instantiate
* 
* Takes a variadic list of Library-Initialization types.
* Will initialize in a RAII style. to be used from main.
*
*/

template<class T,class ... Args>
class Libraries : public Libraries<Args...>
{
private:
	T lib_;
};

template<class T>
class Libraries<T> 
{
private:
	T lib_;
};


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


/**
* \class connection_timeout_t
* \brief timeout defaults
*
*
*/

struct connection_timeout_t
{
	//! connection timeout
	int connect_timeout_s;
	//! read-write timeout
	int rw_timeout_s;
};

/**
* \class Connection
* \brief abstract base of all Connections
*
* base class for TCPConnection and SSLConnection
*/

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	typedef std::shared_ptr<Connection> Ptr;

	virtual ~Connection() {}
   
    //! asynchronously read some available char sequence from socket connection
	virtual repro::Future<Connection::Ptr, std::string> read() = 0;
    //! asynchronously read up to n available chars from socket connection
	virtual repro::Future<Connection::Ptr, std::string> read(size_t n) = 0;
	//! asynchronously write data to socket connection
	virtual repro::Future<Connection::Ptr> write(const std::string& data) = 0;

	//! check if Http2 was requested. Always false if not over SSL.
	virtual bool isHttp2Requested() { return false; }	

	//! close the connection
	virtual void close() = 0;
	//! asynchronous graceful shutdown
	virtual repro::Future<> shutdown() = 0;
	//! cancel current pending IO
	virtual void cancel() = 0;

	virtual std::string common_name() { return ""; }

	//! return connection timeouts for this connection
	virtual connection_timeout_t& timeouts() = 0;

	//! connect a tcp socket to hostname and given port, using HTTP
	static repro::Future<Connection::Ptr> connect(const std::string& host, int port);
	//! connect a tcp socker to hostname and given port, using HTTPS
	static repro::Future<Connection::Ptr> connect(const std::string& host, int port, SslCtx& ctx);
};

/**
* \fn connection_timeouts()
* \brief return the connection timeout defaults
*
*
*/

connection_timeout_t& connection_timeouts();

//////////////////////////////////////////////////////////////
// Ssl Context when using openssl
//////////////////////////////////////////////////////////////

/**
* \class SslCtx
* \brief Openssl context to be used for SSL/TLS
*
* to be used in main()
*/

class SslCtx
{
public:
	SslCtx();
	virtual ~SslCtx();

	//! load TLS certificates from PEM file.
	void load_cert_pem(const std::string& file);
	void verify_certs(bool v);
	bool verify_certs();
	bool verify_client();
	void set_ca_path(const std::string& ca);
	std::string get_ca_path();

	void set_client_ca(const std::string& pem);
	
	std::unique_ptr<SslCtxImpl> ctx;
};

/**
* \fn theSslCtx()
* \brief a default SslCtx()
*
* to be used if you need only one.
*/
SslCtx& theSslCtx();

//////////////////////////////////////////////////////////////
// simple socket listener
// opens and binds a server socket (tcp or ssl)
//////////////////////////////////////////////////////////////

/**
* \class Listener
* \brief  simple socket listener
*
* opens and binds a server socket (tcp or ssl)
*/

class Listener 
{
public:

	Listener();
	Listener(SslCtx& ctx);
	~Listener();

	//! stop listening
	void cancel();

	//! \brief open server socket
	//! bind a tcp/tls socket to port and start accepting incoming connections
	Callback<Connection::Ptr>& bind(int port);

private:

  	std::unique_ptr<ListenerImpl> impl_;
};

//#ifndef _WIN32


/**
* \class IO 
* \brief Wait on file descriptor IO helper
*
* to integrate with 3dparty libs
*/

class IO
{
public:

	IO();
	~IO();

	//! wait for read on given socket
	repro::Future<> onRead(socket_t fd);
	//! wait for write on given socket
	repro::Future<> onWrite(socket_t fd);

	//! wait for read/write on given socket
	repro::Future<short> onSocket(socket_t fd);

	//! cancel any current waits
	void cancel();

private:

    std::unique_ptr<IOImpl> impl_;
};

//#endif


/**
* \brief make an async promise based call for each element of range
* \param Iterator begin
* \param Iterator end
* \param callable F
*
* will resolve its promise once handler has been called for each element.
* or reject on the first exception thrown.
*/

template<class I,class F>
repro::Future<> forEach( I begin, I end, F f )
{
	auto p = repro::promise<>();

	if ( begin == end)
	{
		prio::nextTick([p]()
		{
			p.resolve();
		});
		
		return p.future();
	}

	I step = begin;
	step++;

	f(*begin)
	.then([step,end,f]()
	{
		return forEach(step,end,f);
	})
	.then([p]()
	{
		p.resolve();
	})
	.otherwise(reject(p));

	return p.future();
}

/**
* \brief make an async promise based call for each element of range
* \param Container c
* \param callable F
*
* will resolve its promise once handler has been called for each element.
* or reject on the first exception thrown.
*/

template<class C,class F>
repro::Future<> forEach(C& c, F f )
{
	auto container = std::make_shared<C>(c);

	return forEach( container->begin(), container->end(), [container,f](typename C::value_type i)
	{
		return f(i);
	});
}



} // close namespaces

#endif


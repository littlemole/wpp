#ifndef DEFINE_MOL_HTTP_SERVER_EVENTLOOP_ASIOIMPL_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_EVENTLOOP_ASIOIMPL_DEF_GUARD_DEFINE_

#ifdef PROMISE_USE_BOOST_ASIO

#define _LIBCPP_ENABLE_CXX20_REMOVED_TYPE_TRAITS 1

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "priocpp/api.h"

namespace prio	 	{

//////////////////////////////////////////////////////////////

#ifndef _WIN32
typedef boost::asio::posix::stream_descriptor stream_descriptor;
#endif


class AsioLoop : public Loop
{
public:

	AsioLoop();
	virtual ~AsioLoop() noexcept;

	virtual void run() noexcept;
	virtual void exit()  noexcept;

	virtual void onThreadStart(std::function<void()> f) noexcept;
	virtual void onThreadShutdown(std::function<void()> f) noexcept;

	virtual bool isEventThread() const noexcept;

	virtual repro::Future<int> signal(int s) noexcept;

	boost::asio::io_service& io()
	{
		return io_;
	}

private:

	AsioLoop(const AsioLoop& rhs) = delete;
	AsioLoop(AsioLoop&& rhs) = delete;
	AsioLoop& operator=(const AsioLoop& rhs) = delete;
	AsioLoop& operator=(AsioLoop&& rhs) = delete;

	std::thread::id eventThreadId_;
	std::vector<std::shared_ptr<Signal>> signals_;

	boost::asio::io_service io_;
};


AsioLoop& asioLoop();


class TcpConnectionImpl
{
public:

	TcpConnectionImpl();

	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::resolver resolver;

	enum { max_length = 1024 };
	char data[max_length];

	Timeout timer;

	bool closed = false;
};

class SslConnectionImpl
{
public:

	SslConnectionImpl(SslCtx&);

	bool isHttp2Requested();	
	std::string common_name();

	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket;
	boost::asio::ip::tcp::resolver resolver;
    boost::asio::ssl::context& ctx;

	enum { max_length = 1024 };
	char data[max_length];

	Timeout timer;

	bool closed = false;
};



struct SslCtxImpl
{
	SslCtxImpl();

	boost::asio::ssl::context ssl;

	void verify_certs(bool v);
	bool verify_certs();
	bool verify_client();

	void set_ca_path(const std::string& ca);
	std::string get_ca_path();
	void set_client_ca(const std::string& pem);

	bool verify_;
	bool verify_client_;
	std::string ca_path_;
};

struct ListenerImpl
{	
	ListenerImpl();
	virtual ~ListenerImpl();

    virtual void accept_handler() = 0;

	void bind( int port );
	void cancel();

	boost::asio::ip::tcp::acceptor acceptor;

	Callback<Connection::Ptr> cb;
};



#ifndef _WIN32

struct IOImpl
{
	IOImpl();
	~IOImpl();

	repro::Future<> onRead(socket_t fd);
	repro::Future<> onWrite(socket_t fd);

	void cancel();

private:

	void handle_callback( const repro::Promise<>& p, boost::system::error_code error );

	stream_descriptor sd_;
};

#endif

struct TcpListenerImpl : public ListenerImpl
{
	TcpListenerImpl();	
	~TcpListenerImpl();

    void accept_handler();

	boost::asio::ip::tcp::socket socket;
};



struct SslListenerImpl : public ListenerImpl
{
	SslListenerImpl(SslCtx& ssl);	
	~SslListenerImpl();

    void accept_handler();

	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket;
	SslCtx& ctx;
};

bool is_io_cancelled(const boost::system::error_code& error);

template<class P, class S> 
auto cancellation( P p, S& s)
{
	return [p,&s]() 
	{
		s.cancel();
		p.reject(IoTimeout("socket cancelled due to timeout"));
	};
}

} // close namespaces

#endif
#endif




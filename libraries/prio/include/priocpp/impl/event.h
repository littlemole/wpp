#ifndef DEFINE_MOL_HTTP_SERVER_EVENTLOOP_LIBEVENTIMPL_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_EVENTLOOP_LIBEVENTIMPL_DEF_GUARD_DEFINE_

#ifdef PROMISE_USE_LIBEVENT

#include <fcntl.h>
#include <event2/event.h>
#include <event2/dns.h>

#include "reprocpp/debug.h"
#include "priocpp/api.h"

namespace prio  		{

bool would_block();

//////////////////////////////////////////////////////////////

}

typedef struct bio_st BIO;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;

namespace prio      {


class Event : public std::enable_shared_from_this<Event>
{
public:

	typedef std::shared_ptr<Event> Ptr;
	typedef std::function<void(socket_t fd,short what)> callback_t;

	Event() noexcept;
	~Event();

	static Event::Ptr create(::event_base* loop, socket_t fd = -1, short what=0) noexcept;

	Event::Ptr callback( const callback_t& f);
	Event::Ptr callback( callback_t&& f);
	Event::Ptr add(int secs, int ms) noexcept;
	Event::Ptr add() noexcept;

	void activate(int flags = 0);

	void cancel() noexcept;
	void dispose() noexcept;

	::event* handle();

private:

	callback_t cb_;

	Event(const Event& rhs) = delete;
	Event(Event&& rhs) = delete;

	Event& operator=(const Event& rhs) = delete;
	Event& operator=(Event&& rhs) = delete;

	static void event_handler(socket_t fd, short what, void* arg);

	event* e;
};

Event::Ptr onEvent(socket_t fd, short what);

class LibEventLoop : public Loop
{
public:

	LibEventLoop();
	virtual ~LibEventLoop() noexcept;

	virtual void run() noexcept;
	virtual void exit()  noexcept;

	virtual void onThreadStart(std::function<void()> f) noexcept;
	virtual void onThreadShutdown(std::function<void()> f) noexcept;

	virtual bool isEventThread() const noexcept;

	virtual repro::Future<int> signal(int s) noexcept;

	Event::Ptr on(socket_t fd, short what) const noexcept;

	event_base* base() { return base_; }

private:

	LibEventLoop(const LibEventLoop& rhs) = delete;
	LibEventLoop(LibEventLoop&& rhs) = delete;
	LibEventLoop& operator=(const LibEventLoop& rhs) = delete;
	LibEventLoop& operator=(LibEventLoop&& rhs) = delete;

	std::thread::id eventThreadId_;
	std::vector<std::shared_ptr<Signal>> signals_;

	event_base* base_;
};


LibEventLoop& eventLoop();


class TcpConnectionImpl
{
public:

	TcpConnectionImpl();

	socket_t fd;

	std::shared_ptr<Event> e_read;
	std::shared_ptr<Event> e_write;
};

class SslConnectionImpl
{
public:

	SslConnectionImpl();
	~SslConnectionImpl();

	socket_t fd;
	SSL* ssl;

	std::shared_ptr<Event> e_;

	bool isHttp2Requested();	
	std::string common_name();
};


struct SslCtxImpl
{
	SslCtxImpl();
	~SslCtxImpl();
	
	void loadKeys( const std::string& keyfile );
	void verify_certs(bool v);
	bool verify_certs();
	bool verify_client();

	void set_ca_path(const std::string& ca);
	std::string get_ca_path();
	void set_client_ca(const std::string& pem);
	
	SSL_CTX* ctx;
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

	socket_t fd;
	Event::Ptr e;
	Callback<Connection::Ptr> cb_;
};


struct IOImpl
{
	IOImpl();
	~IOImpl();

	repro::Future<> onRead(socket_t fd);
	repro::Future<> onWrite(socket_t fd);

	void cancel();

	Event::Ptr e;
};



struct TcpListenerImpl : public ListenerImpl
{
	TcpListenerImpl();	
	~TcpListenerImpl();

    void accept_handler();
};



struct SslListenerImpl : public ListenerImpl
{
	SslListenerImpl(SslCtx& ssl);	
	~SslListenerImpl();

    void accept_handler();

	SslCtx& ctx;
};


class Resolver
{
public:

	Resolver();
	~Resolver();

	repro::Future<socket_t> connect(const std::string& host, int port);

private:

	repro::Future<sockaddr_in*> resolve(const std::string host);

};

Resolver& dnsResolver();


} // close namespaces

#endif 
#endif




#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_WS_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_WS_DEF_GUARD_

//! \file websocket.h
//! \defgroup ws

#include <map>
#include <set>

#include <priohttp/request.h>
#include <priohttp/attr.h>

//////////////////////////////////////////////////////////////

namespace prio    {
	class SslCtx;
}

namespace reproweb    {

//! WebSocket COnnection
//! \ingroup ws
class WsConnection : public std::enable_shared_from_this<WsConnection>
{
public:

	prio::Request req;

	~WsConnection();

	typedef std::shared_ptr<WsConnection> Ptr;

	//! \private
	static Ptr create()
	{
		Ptr ptr = Ptr(new WsConnection());
		ptr->self_ = ptr;
		return ptr;
	}

	//! \private
	static Ptr create(prio::SslCtx& ctx)
	{
		Ptr ptr = Ptr(new WsConnection(ctx));
		ptr->self_ = ptr;
		return ptr;
	}

	//! \private
    std::string handshake(prio::Request& r);
	//! \private
    repro::Future<Ptr> connect(std::string urlStr);

	//! send some text over the websocket
    bool send( int opcode, const std::string& s );

	//! attach websocket onMsg handler
    template<class T>
    WsConnection* onMsg( T t )
    {
        onMsg_ = t;
        return this;
    }

	//! attach websocket onClose handler
    template<class T>
    WsConnection* onClose( T t )
    {
        onClose_ = t;
        return this;
    }

	//! attach websocket onConnect handler
    template<class T>
    WsConnection* onConnect( T t )
    {
        onConnect_ = t;
        return this;
    }

	//! \private
    void run()
    {
    	onConnect_(shared_from_this());
		send_msg();
    }

	//! close the websocket
    void close();

	//! \private
	void dispose();

	//! \private
	prio::Connection::Ptr connection();

private:

    WsConnection();
    WsConnection(prio::SslCtx& ctx );

	void onDone();

	void read_frame();
	void read_len();
	void read_mask();
	void read_msg();
	void send_msg();

	std::string unmask( const std::string& data);

	void parse_opcode(unsigned char byte1);
	void parse_fin(unsigned char byte1);
	void parse_mask(unsigned char byte2);
	void parse_len(unsigned char byte2);

	std::string path_;
	std::string msg_;

	std::function<void(Ptr,const std::string&)> onMsg_;
	std::function<void(Ptr)> onConnect_;
	std::function<void(Ptr)> onClose_;
	std::string key_;
	std::string version_;

	bool fin_;
	bool mask_;
	bool isClient_;
	unsigned char opcode_;
	uint64_t size_;
	int extralenbytes_;

	constexpr static size_t keysize_ = 4;
	unsigned char xorkey_[keysize_];

	Ptr self_;
	prio::Connection::Ptr con_;
	prio::SslCtx* ctx_;

	std::deque<std::pair<int,std::string>> msgs_to_send_;

	bool pending_;
};

//////////////////////////////////////////////////////////////////

//! \private
class WebsocketWriter : public std::enable_shared_from_this<WebsocketWriter>
{
public:

	~WebsocketWriter();

	typedef std::shared_ptr<WebsocketWriter> Ptr;

	static Ptr on(int op,WsConnection::Ptr ws, bool isClient)
	{
		Ptr ptr = Ptr( new WebsocketWriter(op,ws,isClient) );
		ptr->self_ = ptr;
		return ptr;
	}

	repro::Future<> write(const std::string& body)
	{
		msg_ = body;
		return write_frame();
	}

private:

    WebsocketWriter(int op, WsConnection::Ptr ws, bool isClient);

    std::string mask( const std::string& buf );

	int opcode_;
	bool isClient_;
	WsConnection::Ptr ws_;

	std::string msg_;
	int framebytes_;
	unsigned char xorkey_[4];

	repro::Future<> write_frame();

	Ptr self_;
};




} // close namespaces

#endif


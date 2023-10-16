#include <iostream>
#include <sstream>

#include "event2/event.h"

#include <reproweb/ws/websocket.h>
#include <priocpp/api.h>
#include <cryptoneat/cryptoneat.h>
#include <cryptoneat/base64.h>
#include "priocpp/connection.h"
#include "priocpp/ssl_connection.h"

#ifdef _WIN32
#include "reproweb/ws/portable_endian.h"
#endif

using namespace prio;
using namespace repro;
using namespace cryptoneat;

namespace reproweb   {


////////////////////////////////////////////////////////////////////////////

const char* WS = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


////////////////////////////////////////////////////////////////////////////


WsConnection::WsConnection()
    :
	  fin_(false),
	  mask_(false),
	  isClient_(false),
	  opcode_(0),
	  size_(0),
	  extralenbytes_(0),
	  ctx_(nullptr),
	  pending_(false)
{
	onConnect_ = [](Ptr){};
	onClose_ = [](Ptr){};
	onMsg_ = [](Ptr,const std::string&){};

	REPRO_MONITOR_INCR(ws);
}


WsConnection::WsConnection( prio::SslCtx& ctx)
    :
	  fin_(false),
	  mask_(false),
	  isClient_(false),
	  opcode_(0),
	  size_(0),
	  extralenbytes_(0),
	  ctx_(&ctx),
	  pending_(false)
{
	onConnect_ = [](Ptr){};
	onClose_ = [](Ptr){};
	onMsg_ = [](Ptr,const std::string&){};

	REPRO_MONITOR_INCR(ws);
}


WsConnection::~WsConnection()
{
	REPRO_MONITOR_DECR(ws);
}


std::string WsConnection::handshake(prio::Request& r)
{
	con_ = r.con();
	path_ = r.path.path();
	req = r;

    std::string upgrade(req.headers.get("Upgrade"));
    std::string connection = req.headers.get("Connection");
    key_ = req.headers.get("Sec-WebSocket-Key");
    version_ = req.headers.get("Sec-WebSocket-Version");

    if ( upgrade != std::string("websocket") )
    {
        std::cerr << "handshake failed upgrade *" << upgrade << "*" << std::endl;
        return "";
    }

    size_t pos = connection.find("Upgrade");
    if ( pos == std::string::npos )
    {
        std::cerr << "handshake failed connection" << std::endl;
        return "";
    }

    if ( key_.empty() )
    {
        std::cerr << "handshake failed key" << std::endl;
        return "";
    }

    if ( version_.empty() )
    {
        std::cerr << "handshake failed version" << std::endl;
        return "";
    }

    std::ostringstream hss;
    hss << key_ << WS;

    std::string hash = Base64::encode( sha1(hss.str()) );

    if(!hash.empty())
    {
    	HttpRequest& request = (HttpRequest&)r;
    	request.detach();
		request.con()->timeouts().rw_timeout_s = 1000L * 60L * 10L;
    }
    else {
    	dispose();
    }

    return hash;
}

Future<WsConnection::Ptr> WsConnection::connect(std::string urlStr)
{
	auto p = repro::promise<WsConnection::Ptr>();
	isClient_ = true;

	Url url(urlStr);

	Future<Connection::Ptr> f;

	if ( url.getProto() == "ws")
	{
		f = TcpConnection::connect(url.getHost(),url.getPort());
	}
	else if ( url.getProto() == "wss" && ctx_ )
	{
		f = SslConnection::connect( url.getHost(),url.getPort(),*ctx_);
	}
	else
	{
		nextTick([p]()
		{
			p.reject(repro::Ex("invalid websocket protocol"));
		});
		return p.future();
	}


	f.then([this,url](Connection::Ptr client)
	{
		con_ = client;
		std::string key = "dGhlIHNhbXBsZSBub25jZQ=="; // something in base64
		std::string ver = "13";

		std::ostringstream oss;
		oss << "GET " << url.getPath() << " HTTP/1.1\r\n"
		<< "Upgrade: websocket\r\n"
		<< "Connection: Keep-Alive,Upgrade\r\n"
		<< "User-Agent: molws\r\n"
		<< "Host: " << url.getHost() << "\r\n"
		<< "Sec-WebSocket-Key: " << key << "\r\n"
		<< "Sec-WebSocket-Version: " << ver << "\r\n\r\n";

		auto& t = client->timeouts();
		t.rw_timeout_s = 3600 * 1000;

		//std::cout << "WS REQ: " << oss.str() << std::endl;

		return client->write(oss.str());
	})
	.then([](Connection::Ptr client)
	{
		return client->read();
	})
	.then([this,p](Connection::Ptr client, std::string data)
	{
		//std::cout << "WS DATA: " << data << std::endl;

		//TODO: check ws handshake response ?
		auto tmp = p;
		tmp.resolve(this->shared_from_this());
		send_msg();
	})	
	.otherwise( [this,p](const std::exception_ptr& ex) {

		p.reject(ex);
		onClose_(shared_from_this());
		dispose();
	});

	return p.future();
}

void WsConnection::parse_opcode(unsigned char byte1)
{
    int oc = ( (byte1) & 0x0F);

    if ( oc != 0 )
    {
        opcode_ = oc;
    }
}

void WsConnection::parse_fin(unsigned char byte1)
{
	fin_ = (((byte1) >> 7) & 0x01);
}


void WsConnection::parse_mask(unsigned char byte2)
{
	mask_ = (((byte2) >> 7) & 0x01);
}

void WsConnection::parse_len(unsigned char byte2)
{
    unsigned char len =  byte2;

    unsigned int l = len;
	if (!isClient_)
	{
		l -= 128;
	}
    if ( l <= 125 )
    {
        size_ = l;
    }
    else if ( l == 126 )
    {
        extralenbytes_ = 2;
    }
    else if ( l == 127 )
    {
        extralenbytes_ = 8;
    }
}


void WsConnection::read_frame()
{
	con_
	->read(2)
	.then([this](Connection::Ptr,std::string data)
	{
		pending_ = true;
		extralenbytes_ = 0;
		unsigned char byte1 = *(unsigned char*)(data.c_str());
		unsigned char byte2 = *(unsigned char*)(data.c_str()+1);

		parse_fin(byte1);
		parse_opcode(byte1);

		if ( opcode_ == 0x08 )
		{
			onClose_(shared_from_this());
			dispose();
			return;
		}

		parse_mask(byte2);
		parse_len(byte2);

		read_len();
	})
	.otherwise( [this](const std::exception_ptr& e)
	{
		pending_ = false;
		onClose_(shared_from_this());
		dispose();
	});

	return;
}

void WsConnection::read_len()
{
	auto read_mask_or_msg = [this]() 
	{
		if( mask_ )
		{
			read_mask();
		}
		else
		{
			read_msg();
		}
	};

	if ( extralenbytes_ != 0 )
	{
		con_
		->read(extralenbytes_)
		.then([this,read_mask_or_msg](Connection::Ptr,std::string data)
		{
			if ( extralenbytes_ == 2 )
			{
				uint16_t s = *(uint16_t*)(data.c_str());
				size_ = be16toh(s);
			}
			else
			{
				uint64_t s = *(uint64_t*)(data.c_str());
				size_ = be64toh(s);
			}

			read_mask_or_msg();
		})
		.otherwise([this](const std::exception_ptr& ex)
		{
			pending_ = false;
		});
	}
	else
	{
		read_mask_or_msg();
	}
}

void WsConnection::read_mask()
{
	con_
	->read(4)
	.then([this](Connection::Ptr,std::string data)
	{
		memcpy(xorkey_,data.c_str(),4);
		read_msg();
	})
	.otherwise([](const std::exception& ex)
	{
		std::cout << "read_mask failed" << std::endl;
		std::cout << ex.what() << std::endl;
	});
}

std::string WsConnection::unmask( const std::string& buf )
{
	std::ostringstream oss;
    for ( size_t i = 0; i < buf.size(); i++ )
    {
        unsigned char c = (unsigned char) buf[i];
        unsigned char decoded = c ^ (xorkey_[i%4]);

        oss.write( (char*) &decoded,1);
    }
    return oss.str();
}


void WsConnection::read_msg()
{

	con_
	->read((size_t)size_)
	.then([this](Connection::Ptr,std::string data)
	{
		pending_ = false;
		if(isClient_)
		{
			msg_.append(data);
		}
		else
		{
			msg_.append( unmask( data ) );
		}

		if ( fin_ )
		{
			onDone();
		}
		else if(con_)
		{
			read_frame();
		}
	})
	.otherwise([this](const std::exception& ex){

		pending_ = false;

		std::cout << "read_msg failed" << std::endl;
		std::cerr << ex.what() << std::endl;
		self_.reset();
	});
}

bool WsConnection::send( int opcode, const std::string& s )
{
	if(!con_)
	{
		return false;
	}

	msgs_to_send_.push_back(std::make_pair(opcode, s));
	if(!pending_)
	{
		send_msg();
		return true;
	}

	return false;
}


void WsConnection::send_msg( )
{
	if(pending_)
	{
		return;
	}

	if(!con_)
	{
		return;
	}

	con_->cancel();

	if(msgs_to_send_.empty())
	{
		read_frame();
		return;
	}

	pending_ = true;
	auto m = msgs_to_send_.front();
	msgs_to_send_.pop_front();

	WebsocketWriter::on(m.first,shared_from_this(),isClient_)
	->write(m.second)
	.then([this]()
	{
		pending_ = false;
		send_msg();
	})
	.otherwise([this](const std::exception& ex)
	{
		pending_ = false;
		std::cout << "send_msg failed" << std::endl;
		std::cout << ex.what() << std::endl;
		send_msg();
	});	
}

void WsConnection::onDone()
{
    if ( opcode_ == 0x09 ) // PING
    {
        send(0x0A,msg_);   // PONG
    }
    else
    {
		onMsg_(shared_from_this(),msg_);
		msg_.clear();

		send_msg();
    }
}

void WsConnection::close()
{
	send( 0x8, "");
}

void WsConnection::dispose()
{
	if(con_)
	{
		con_->cancel();
		con_->close();
		con_.reset();
	}
	self_.reset();
}

Connection::Ptr WsConnection::connection()
{
	return con_;
}

////////////////////////////////////////////////////////////////////////////


WebsocketWriter::WebsocketWriter(int op, WsConnection::Ptr f, bool isClient)
 : opcode_(op), isClient_(isClient), ws_(f), framebytes_(2),xorkey_{ 0xFF,0x11,0xFF,0x11 }
{
	if (isClient)
	{
		for ( int i = 0; i < 4; i++) {

			int number = 1;// TODO RAND!
			xorkey_[i] ^= number;
		}
	}

	REPRO_MONITOR_INCR(wsWriter);
}

WebsocketWriter::~WebsocketWriter()
{
	REPRO_MONITOR_DECR(wsWriter);
}

std::string WebsocketWriter::mask( const std::string& buf )
{
	std::ostringstream oss;
    for ( size_t i = 0; i < buf.size(); i++ )
    {
        unsigned char c = (unsigned char) buf[i];
        unsigned char decoded = c ^ (xorkey_[i%4]);

        oss.write( (char*) &decoded,1);
    }

    return oss.str();
}

Future<> WebsocketWriter::write_frame()
{
	auto p = repro::promise<>();

    char buf[14];
    buf[0] = (char)(0x01 << 7); // FIN

    buf[0] = buf[0] | opcode_; // OPCODE

    buf[1] = 0; // LEN

    if ( msg_.size() < 126 )
    {
        buf[1] = (uint8_t)msg_.size();
    }
    else if ( msg_.size() < UINT16_MAX )
    {
        framebytes_ = 4;
        buf[1] = 126;
        uint16_t l = (uint16_t)msg_.size();
        *(uint16_t*)(buf+2) = htobe16(l);
    }
    else
    {
        framebytes_ = 10;
        buf[1] = 127;
        uint64_t l = (uint64_t)msg_.size();
        *(uint64_t*)(buf+2) = htobe64(l);
    }

	if (isClient_)
	{
    	buf[1] |= (0x01 << 7);

		for( int i = 0; i < 4; i++)
		{
			(buf[framebytes_+i]) = xorkey_[i];
		}
		framebytes_ += 4;
		msg_ = mask(msg_);
	}

	std::ostringstream payload;
    payload.write(buf,framebytes_);
	payload << msg_;

	ws_->connection()
	->write(payload.str())
	.then([p,this](Connection::Ptr client)
	{
		p.resolve();
		self_.reset();
	})
	.otherwise( [this,p](const std::exception& e)
	{
		p.reject(repro::Ex(e.what()));
		self_.reset();
	});

	return p.future();
}


////////////////////////////////////////////////////////////////////////////

} // close namespaces


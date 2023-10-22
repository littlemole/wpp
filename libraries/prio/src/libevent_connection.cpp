#ifdef PROMISE_USE_LIBEVENT


#include <fcntl.h>
#include "reprocpp/debug.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include "priocpp/connection.h"
#include "priocpp/impl/event.h"

#ifdef _WIN32
#define SHUT_RDWR SD_SEND 
#define read_socket(s,b,l) ::recv(s,b,l,0);
#define write_socket(s,b,l) ::send(s,b,l,0);
#define close_socket ::closesocket
#else 
#define read_socket(s,b,l) ::read(s,b,l);
#define write_socket(s,b,l) ::write(s,b,l);
#define close_socket ::close
#endif

using namespace repro;

namespace prio      {


TcpConnectionImpl::TcpConnectionImpl()
	: fd(-1)
{
}

TcpConnection::TcpConnection(TcpConnectionImpl* impl)
	: impl_(impl), timeouts_(connection_timeouts())
{
	REPRO_MONITOR_INCR(TcpConnection);
}

TcpConnection::~TcpConnection()
{
	close();
	REPRO_MONITOR_DECR(TcpConnection);
}

connection_timeout_t& TcpConnection::timeouts()
{
	return timeouts_;
}

Future<Connection::Ptr> TcpConnection::connect(const std::string& host, int port)
{
	auto p = promise<Connection::Ptr>();

	auto impl = new TcpConnectionImpl;
	Connection::Ptr ptr( new TcpConnection(impl) );		

	dnsResolver()
	.connect(host,port)
	.then( [p,impl,ptr](socket_t fd)
	{
		impl->fd = fd;
		p.resolve(ptr);
	})
	.otherwise([p](const std::exception& ex)
	{
		p.reject(ex);
	});

	return p.future();
}



Future<Connection::Ptr, std::string> TcpConnection::read()
{
	auto p = promise<Connection::Ptr,std::string>();
	impl_->e_read = onEvent(impl_->fd,(short)EV_READ)
	->callback( [p,this] (socket_t fd, short what)
	{
		if( what & EV_TIMEOUT)
		{
			p.reject( IoTimeout("timeout in TcpConnection::read") );
			return;
		}

		char buf[1024];
		auto len = read_socket(fd,buf,1024);

		if(len > 0)
		{
			std::string data(buf,len);
			p.resolve(shared_from_this(),data);
		}
		else if(len==0)
		{
			p.reject( Ex("EOF in Event.read") );
		}
		else
		{
			if(would_block())
			{
				impl_->e_read->add(timeouts_.rw_timeout_s,0);
			}
			else
			{
				p.reject(Ex("IO ex in Event.read"));
			}
		}
	});

	if (timeouts_.rw_timeout_s != 0)
	{
		impl_->e_read->add(timeouts_.rw_timeout_s, 0);
	}
	else
	{
		impl_->e_read->add();
	}

	return p.future();
}

Future<Connection::Ptr, std::string> TcpConnection::read(size_t s)
{
	auto p = promise<Connection::Ptr,std::string>();

	std::shared_ptr<size_t> want = std::make_shared<size_t>(s);
	std::shared_ptr<std::string> buffer = std::make_shared<std::string>();

	impl_->e_read = onEvent(impl_->fd,EV_READ)
	->callback( [p,this,buffer,want,s](socket_t fd, short what)
	{
		if( what & EV_TIMEOUT)
		{
			p.reject( IoTimeout("timeout in TcpConnection::read") );
			return;
		}

		std::vector<char> buf(s, 0);
		long len  = read_socket( fd, &buf[0], (int)*want );

		if ( len <= 0 )
		{
			if(would_block())
			{
				impl_->e_read->add(timeouts_.rw_timeout_s,0);
			}
			else
			{
				p.reject(Ex("IOex in Event.send"));
			}
			return;
		}

		buffer->append(std::string(&buf[0],len));

		if(*want -len <=0 )
		{
			p.resolve(shared_from_this(),*buffer);
		}
		else
		{
			*want -= len;
			impl_->e_read->add(timeouts_.rw_timeout_s,0);
		}
	});

	if (timeouts_.rw_timeout_s != 0)
	{
		impl_->e_read->add(timeouts_.rw_timeout_s, 0);
	}
	else
	{
		impl_->e_read->add();
	}

	return p.future();
}

Future<Connection::Ptr> TcpConnection::write(const std::string& data)
{
	auto p = promise<Connection::Ptr>();

	std::shared_ptr<size_t> written = std::make_shared<size_t>(0);

	while(true)
	{
		long n  = write_socket( impl_->fd, data.c_str() + *written, (int)( data.size() - *written) );
		if ( n < 0 )
		{
			if(would_block())
			{
				// stop writing synchronously
				break;
			}

			nextTick([p]()
			{
				p.reject(Ex("IO ex in Event.send"));
			});
			return p.future();
		}
		*written += n;

		if(*written >= data.size())
		{
			nextTick([this,p]()
			{
				p.resolve(shared_from_this());
			});
			return p.future();
		}
	}

	// start waiting for write

	impl_->e_write = onEvent(impl_->fd,EV_WRITE)
	->callback( [p,this,data,written](socket_t /* fd */, short what)
	{
		if( what & EV_TIMEOUT)
		{
			p.reject( IoTimeout("timeout in TcpConnection::read") );
			return;
		}

		long n  = write_socket( impl_->fd, data.c_str() + *written, (int)( data.size() - *written) );

		if ( n < 0 )
		{
			if(would_block())
			{
				impl_->e_write->add(timeouts_.rw_timeout_s,0);
			}
			else
			{
				p.reject(Ex("IOex in Event.send"));
			}
			return;
		}

		*written += n;

		if(*written >= data.size())
		{
			p.resolve(shared_from_this());
		}
		else
		{
			impl_->e_write->add(timeouts_.rw_timeout_s,0);
		}
	});

	if (timeouts_.rw_timeout_s != 0)
	{
		impl_->e_write->add(timeouts_.rw_timeout_s, 0);
	}
	else
	{
		impl_->e_write->add();
	}

	return p.future();
}

void TcpConnection::close()
{
	cancel();
	if(impl_->fd != -1)
	{
		close_socket(impl_->fd);
		impl_->fd = -1;
	}
}

Future<> TcpConnection::shutdown()
{
	auto p = promise<>();

	if(impl_->fd == -1)
		return p.resolved();

	::shutdown(impl_->fd,SHUT_RDWR);
	read()
	.then([this](Connection::Ptr,std::string /* data */)
	{
		close();
	})
	.otherwise([this](const std::exception& /* ex */)
	{
		close();
	});

	return p.future();
}

void TcpConnection::cancel()
{
	if(impl_ && impl_->e_read)
	{
		impl_->e_read->cancel();
	}

	if(impl_ && impl_->e_write)
	{
		impl_->e_write->cancel();
	}
}



} // close namespaces



#endif

#ifdef PROMISE_USE_BOOST_ASIO

#include "priocpp/impl/asio.h"
#include "priocpp/api.h"

using namespace repro;

namespace prio      {


struct TimeoutImpl
{
	TimeoutImpl()
	{}

	std::shared_ptr<boost::asio::deadline_timer> timer;
};

Timeout::Timeout()
: impl_(new TimeoutImpl)
{
	REPRO_MONITOR_INCR(timeout);
}

Timeout::~Timeout()
{
	cancel();
	REPRO_MONITOR_DECR(timeout);

}

Future<> Timeout::after(int ms)
{
	auto p = promise<>();

	cancel();
	impl_->timer = std::make_shared<boost::asio::deadline_timer>(
		asioLoop().io(), 
		boost::posix_time::milliseconds(ms)
	);

	impl_->timer
	->async_wait( [p](const boost::system::error_code& e) 
	{
		if(e)
		{
			if(e != boost::asio::error::operation_aborted)
			{
				p.reject(Ex("boost timer error"));
			}
		}
		else
		{
			p.resolve();
		}
	});

	return p.future();
}

void Timeout::after( const std::function<void()>& f, int ms)
{
	cancel();
	impl_->timer = std::make_shared<boost::asio::deadline_timer>(
		asioLoop().io(), 
		boost::posix_time::milliseconds(ms)
	);

	impl_->timer
	->async_wait( [f](const boost::system::error_code& e) 
	{
		if(!e)
		{
			f();
		}
	});
}

void Timeout::after( std::function<void()>&& f, int ms)
{
	cancel();
	impl_->timer = std::make_shared<boost::asio::deadline_timer>(
		asioLoop().io(), 
		boost::posix_time::milliseconds(ms)
	);

	impl_->timer
	->async_wait( [f](const boost::system::error_code& e) 
	{
		if(!e)
		{
			f();
		}
	});
}

void Timeout::cancel()
{
	if(impl_ && impl_->timer)
	{
		impl_->timer->cancel();
	}
}



//////////////////////////////////////////////////////////////

} // close namespaces

#endif

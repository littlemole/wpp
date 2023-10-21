#ifdef PROMISE_USE_BOOST_ASIO

#include "priocpp/impl/asio.h"
#include "priocpp/api.h"

using namespace repro;

namespace prio      {

struct SignalImpl
{
	SignalImpl()
	: signals(asioLoop().io())
	{}

	boost::asio::signal_set signals;
};

Signal::Signal()
	: impl_(new SignalImpl)
{}

Signal::~Signal()
{}


Future<int> Signal::when(int s)
{
	auto p = promise<int>();

	impl_->signals.add( s);

	wait(p,s);

	return p.future();
}


void Signal::wait(Promise<int> p, int /*s*/ )
{
	impl_->signals.async_wait( [this,p]( const boost::system::error_code& e,int signal_number)
	{
		if(e)
		{
			return;
		}

		p.resolve(signal_number);
		wait(p,signal_number);
	});		
}


void Signal::cancel()
{
	impl_->signals.cancel();
	impl_->signals.clear();
}

//////////////////////////////////////////////////////////////

} // close namespaces

#endif

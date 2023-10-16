#ifdef PROMISE_USE_LIBEVENT


#include "priocpp/api.h"
#include "priocpp/impl/event.h"

using namespace repro;

namespace prio      {

struct TimeoutImpl
{
	TimeoutImpl()
	{}

	Event::Ptr e_timer;
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
	int s = ms / 1000;
	ms = ms - (s*1000);

	cancel();
	auto p = promise();
	impl_->e_timer = onEvent(-1,0);
	impl_->e_timer->callback( [p](int fd, short what)
	{
		p.resolve();
	});
	impl_->e_timer->add(s,ms);

	return p.future();
}

void Timeout::after( const std::function<void()>& f, int ms)
{
	int s = ms / 1000;
	ms = ms - (s*1000);

	cancel();

	impl_->e_timer = onEvent(-1,0);
	impl_->e_timer->callback( [f](int fd, short what)
	{	
		f();
	});
	impl_->e_timer->add(s,ms);
}


void Timeout::after( std::function<void()>&& f, int ms)
{
	int s = ms / 1000;
	ms = ms - (s * 1000);

	cancel();

	impl_->e_timer = onEvent(-1, 0);
	impl_->e_timer->callback([f](int fd, short what)
	{
		f();
	});
	impl_->e_timer->add(s, ms);
}


void Timeout::cancel()
{
	if( impl_ && impl_->e_timer.get())
	{
		impl_->e_timer->cancel();
	}
}



//////////////////////////////////////////////////////////////

} // close namespaces

#endif

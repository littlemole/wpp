#ifdef PROMISE_USE_LIBEVENT


#include "priocpp/api.h"
#include "priocpp/impl/event.h"

using namespace repro;

namespace prio      {


struct SignalImpl
{
	SignalImpl()
	{}

	Event::Ptr e_signal;
};

Signal::Signal()
	: impl_(new SignalImpl)
{}

Signal::~Signal()
{}


Future<int> Signal::when(int s)
{
	auto p = promise<int>();

	impl_->e_signal = onEvent( s, EV_SIGNAL|EV_PERSIST);
	impl_->e_signal->callback( [p](int fd, short what) {

		p.resolve(fd);
	});
	impl_->e_signal->add();

	return p.future();
}


void Signal::wait(Promise<int> p, int s)
{

}


void Signal::cancel()
{
	impl_->e_signal->cancel();
}

//////////////////////////////////////////////////////////////

} // close namespaces

#endif

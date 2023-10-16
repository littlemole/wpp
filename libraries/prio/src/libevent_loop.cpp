#ifdef PROMISE_USE_LIBEVENT


#include "priocpp/api.h"
#include "priocpp/impl/event.h"

using namespace repro;

namespace prio      {


std::thread::id null_thread;


Loop& theLoop()
{
	return eventLoop();
}



LibEventLoop& eventLoop()
{
	static LibEventLoop loop;
	return loop;
}


LibEventLoop::LibEventLoop()
{
	base_ = event_base_new();
	eventThreadId_ = std::this_thread::get_id();
}

LibEventLoop::~LibEventLoop()  noexcept
{
	::event_base_free(base_);
}

Event::Ptr LibEventLoop::on(socket_t fd, short what) const noexcept
{
	return Event::create(base_,fd,what);
}

void LibEventLoop::run()  noexcept
{
	thePool().start();
	event_base_dispatch(base_);

	signals_.clear();
}

void LibEventLoop::exit()  noexcept
{
	for( std::shared_ptr<Signal> s : signals_)
	{
		s->cancel();
	}
	nextTick( [this] {
		thePool().stop();
		event_base_loopbreak(base_);
	});	
}


void LibEventLoop::onThreadStart(std::function<void()> f) noexcept
{
	thePool().onThreadStart = f;
}

void LibEventLoop::onThreadShutdown(std::function<void()> f) noexcept
{
	thePool().onThreadShutdown = f;
}

bool LibEventLoop::isEventThread() const noexcept
{
	return eventThreadId_ == std::this_thread::get_id();
}

Future<int> LibEventLoop::signal(int s) noexcept
{
	auto p = promise<int>();

	auto signal = std::make_shared<Signal>();
	signals_.push_back(signal);

	signal->when(s)
	.then( [p]( int signal_number)
	{
		p.resolve(signal_number);
	});		

	return p.future();
}

//////////////////////////////////////////////////////////////

} // close namespaces

#endif

#ifdef PROMISE_USE_BOOST_ASIO

#include "priocpp/impl/asio.h"
#include "priocpp/api.h"

using namespace repro;

namespace prio      {


std::thread::id null_thread;


Loop& theLoop()
{
	return asioLoop();
}



AsioLoop& asioLoop()
{
	static AsioLoop loop;
	return loop;
}


AsioLoop::AsioLoop()
{
	eventThreadId_ = std::this_thread::get_id();
}

AsioLoop::~AsioLoop()  noexcept
{
}

void AsioLoop::run()  noexcept
{
	thePool().start();
	io_.run();
	io_.reset();
	signals_.clear();
}

void AsioLoop::exit()  noexcept
{
	for( std::shared_ptr<Signal> s : signals_)
	{
		s->cancel();
	}
	nextTick( [this] {
		thePool().stop();
		io_.stop();
	});
}


void AsioLoop::onThreadStart(std::function<void()> f) noexcept
{
	thePool().onThreadStart = f;
}

void AsioLoop::onThreadShutdown(std::function<void()> f) noexcept
{
	thePool().onThreadShutdown = f;
}

bool AsioLoop::isEventThread() const noexcept
{
	return eventThreadId_ == std::this_thread::get_id();
}

Future<int> AsioLoop::signal(int s) noexcept
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

#ifndef DEFINE_MOL_HTTP_SERVER_EVENTLOOP_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP_SERVER_EVENTLOOP_DEF_GUARD_DEFINE_

#include "priocpp/threadpool.h"

/**
 * \file loop.h
 */

namespace prio  		{

//////////////////////////////////////////////////////////////

/**
 * \brief main event loop
 *
 * runs the basic eventloop, powered by libevent or boost::asio.
 * the event loop model enforced is one loop per process.
 */
class Loop
{
public:

	virtual ~Loop() noexcept {};

	//! run the loop
	virtual void run() noexcept = 0;
	//! exit the loop
	virtual void exit()  noexcept = 0;

	//! specify per-thread initializer lambda (to initialize libraries on the threadpool threads)
	virtual void onThreadStart(std::function<void()> f) noexcept = 0;
	//! specify per-thread de-initializer lambdas
	virtual void onThreadShutdown(std::function<void()> f) noexcept = 0;

	//! determine whether we are on the event thread
	virtual bool isEventThread() const noexcept = 0;

	virtual repro::Future<int> signal(int s) noexcept = 0;
};

//! return the one and only main event loop.
Loop& theLoop();



} // close namespaces


#endif




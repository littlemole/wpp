#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_HANDLER_TASK_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_HANDLER_TASK_DEF_GUARD_

/**
 * \file task.h
 */

#include <atomic>

#include "priocpp/api.h"
#include "priocpp/threadpool.h"

namespace prio  		{

/// @private
inline void task_synchronize_thread(std::atomic<bool>& running)
{
	bool d = running.load();
	while (d)
	{
		bool expected = true;
		if (running.compare_exchange_weak(expected, false))
		{
			break;
		}
		d = running.load();
	}
}

/// @private
inline void task_synchronize_main(std::atomic<bool>& running)
{
	bool d = running.load();
	while (d)
	{
		d = running.load();
	}	
}


template<class T>
class Task
{};


template<class R,class T>
class Task<R(T)>
{
public:
	static repro::Future<R> exec(T t, ThreadPool& pool  )
	{
		auto p = repro::promise<R>();
		auto running = std::make_shared<std::atomic<bool>>(true);

		nextTick( [t,p,running,&pool]()
		{
			pool.enqueue( [t,p,running]()
			{
				try
				{
					std::shared_ptr<R> r = std::make_shared<R>(t());

					task_synchronize_thread(*running);
					
					nextTick( [p,r,running] ()
					{
						task_synchronize_main(*running);

						p.resolve(std::move(*r));
					});
				}
				catch(...)
				{
					std::exception_ptr eptr = std::current_exception();

					task_synchronize_thread(*running);

					nextTick( [p,eptr,running] ()
					{
						task_synchronize_main(*running);
						p.reject(eptr);
					});
				}
			});
		});
		return p.future();
	}
};


template<class T>
class Task<void(T)>
{
public:

	static repro::Future<> exec(T t, ThreadPool& pool  )
	{
		auto p = repro::promise<>();
		auto running = std::make_shared<std::atomic<bool>>(true);

		nextTick( [t,p,running,&pool]()
		{
			pool.enqueue( [t,p,running]()
			{
				try
				{
					t();

					task_synchronize_thread(*running);

					nextTick( [p,running] ()
					{
						task_synchronize_main(*running);

						p.resolve();
					});
				}
				catch(...)
				{
					std::exception_ptr eptr = std::current_exception();

					task_synchronize_thread(*running);

					nextTick( [p,eptr,running] ()
					{
						task_synchronize_main(*running);

						p.reject(eptr);
					});
				}
			});
		});
		return p.future();
	}
};


/**
 * \brief asynchronous task executed on threadpool.
 * must return void or a single value-type holding result
 * param T is a lambda with no arguments returning void or the single result
 * after completion of T on the threadpool control will be passed back
 * to main event thread and the return future will be called
 * with the result from the task, if any.
 */

template<class T, typename std::enable_if<!repro::traits::returns_void<T>::value>::type* = nullptr>
auto task(T t, ThreadPool& pool = thePool() ) -> repro::Future<decltype(t())>
{
	return Task<decltype(t())(T)>::exec(t,pool);
}

template<class T, typename std::enable_if<repro::traits::returns_void<T>::value>::type* = nullptr>
auto task(T t, ThreadPool& pool = thePool() ) -> repro::Future<>
{
	return Task<void(T)>::exec(t,pool);
}



} // close namespaces

#endif


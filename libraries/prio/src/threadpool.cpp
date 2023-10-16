#include "priocpp/threadpool.h"


namespace prio 		{

namespace impl 		{

//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////

class Worker
{
public:

	Worker(ThreadPool& p)
		: pool_(p)
	{ }

    void operator()()const noexcept ;

private:

    ThreadPool& pool_;
};

} // end namespace impl

ThreadPool& thePool()
{
	static ThreadPool pool(4);
	return pool;
}	

ThreadPool::ThreadPool(size_t threads)
    :
	  onThreadStart([]{}),
	  onThreadShutdown([]{}),
	  stop_(false),
  	  nthreads_(threads)
{}

ThreadPool::~ThreadPool() noexcept
{
	stop();
}

void ThreadPool::start() noexcept
{
	stop_.store(false);
    for( int i = 0; i < nthreads_; ++i )
    {
        workers_.push_back(std::thread(impl::Worker(*this)));
    }
}

void ThreadPool::stop() noexcept
{
	bool b = stop_.load();
	if ( b )
	{
		return;
	}
    // stop all threads
    stop_.store(true);
    condition_.notify_all();

    // join them
    for( size_t i = 0; i < workers_.size(); ++i )
    {
        workers_[i].join();
    }
    workers_.clear();
}


namespace impl {

void Worker::operator()() const noexcept
{
	try
	{
		pool_.onThreadStart();
		while(true)
		{
			std::function<void()> task;
			{   // acquire lock
				std::unique_lock<std::mutex>
					lock(pool_.queue_mutex_);

				bool b = pool_.stop_.load();
				while(!b && pool_.tasks_.empty())
				{
					pool_.condition_.wait(lock);
					b = pool_.stop_.load();
				}

				if(b)
					break;

				task = pool_.tasks_.front();
				pool_.tasks_.pop_front();

			}   // release lock

			task();
		}
		pool_.onThreadShutdown();
	}
	catch(...)
	{
		std::terminate();
	}
}

}


} // close namespaces


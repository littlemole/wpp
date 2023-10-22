#ifndef INCLUDE_EVE_THREADPOOL_H_
#define INCLUDE_EVE_THREADPOOL_H_

#include "priocpp/common.h"
#include <atomic>

//////////////////////////////////////////////////////////////

namespace prio  		{

namespace impl      {
    class Worker;
}

//////////////////////////////////////////////////////////////

class ThreadPool
{
public:

    ThreadPool(size_t);
    ~ThreadPool() noexcept;

    template<class F>
    void enqueue(F f) noexcept;

    void start() noexcept;
    void stop() noexcept;

    std::function<void()> onThreadStart;
    std::function<void()> onThreadShutdown;

private:

    friend class prio::impl::Worker;

    std::vector< std::thread > workers_;

    std::deque< std::function<void()> > tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t nthreads_;

    ThreadPool() = delete;
    ThreadPool(const ThreadPool& rhs) = delete;
    ThreadPool(ThreadPool&& rhs) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
    ThreadPool& operator=(ThreadPool&& rhs) = delete;
};

//////////////////////////////////////////////////////////////

template<class F>
void ThreadPool::enqueue(F f) noexcept
{
    { // acquire lock
        std::unique_lock<std::mutex> lock(queue_mutex_);

        tasks_.push_back(std::function<void()>(f));
    } // release lock

    condition_.notify_one();
}

//////////////////////////////////////////////////////////////

ThreadPool& thePool();

} // close namespaces


#endif 

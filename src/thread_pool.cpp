#include "thread_pool.h"

namespace fmf
{

ThreadPool::ThreadPool(size_t numThreads) : stop_(false), activeTasks_(0)
{
    // Ensure at least one thread
    if (numThreads == 0)
    {
        numThreads = 1;
    }

    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
    {
        workers_.emplace_back([this] { workerThread(); });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }

    condition_.notify_all();

    for (std::thread& worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

size_t ThreadPool::size() const { return workers_.size(); }

size_t ThreadPool::pendingTasks() const
{
    std::unique_lock<std::mutex> lock(queueMutex_);
    return tasks_.size();
}

void ThreadPool::wait()
{
    std::unique_lock<std::mutex> lock(queueMutex_);
    condition_.wait(lock,
                    [this] { return tasks_.empty() && activeTasks_ == 0; });
}

void ThreadPool::workerThread()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty())
            {
                return;
            }

            if (!tasks_.empty())
            {
                task = std::move(tasks_.front());
                tasks_.pop();
                activeTasks_++;
            }
        }

        if (task)
        {
            task();
            activeTasks_--;
            condition_.notify_all();
        }
    }
}

}  // namespace fmf

#ifndef FMF_THREAD_POOL_H
#define FMF_THREAD_POOL_H

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace fmf
{

/**
 * @brief Thread pool for parallel task execution
 *
 * Manages a pool of worker threads that execute tasks from a queue.
 * Supports arbitrary task types via std::function and returns results via
 * std::future.
 */
class ThreadPool
{
 public:
    /**
     * @brief Construct thread pool with specified number of threads
     * @param numThreads Number of worker threads (default: hardware
     * concurrency)
     */
    explicit ThreadPool(
        size_t numThreads = std::thread::hardware_concurrency());

    /**
     * @brief Destructor - waits for all tasks to complete
     */
    ~ThreadPool();

    // Delete copy and move constructors/operators
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * @brief Submit a task to the thread pool
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Arguments to pass to function
     * @return Future containing the result
     */
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Get the number of worker threads
     * @return Number of threads in pool
     */
    size_t size() const;

    /**
     * @brief Get the number of pending tasks
     * @return Number of tasks in queue
     */
    size_t pendingTasks() const;

    /**
     * @brief Wait for all tasks to complete
     */
    void wait();

 private:
    /**
     * @brief Worker thread function
     */
    void workerThread();

    // Worker threads
    std::vector<std::thread> workers_;

    // Task queue
    std::queue<std::function<void()>> tasks_;

    // Synchronization
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<size_t> activeTasks_;
};

// Template implementation
template <typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using ReturnType = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<ReturnType> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if (stop_)
        {
            throw std::runtime_error(
                "Cannot submit task to stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }

    condition_.notify_one();
    return result;
}

}  // namespace fmf

#endif  // FMF_THREAD_POOL_H

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "infrastructure/threading/thread_pool.h"

using namespace fmf;

class ThreadPoolTest : public ::testing::Test
{
 protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ThreadPoolTest, ConstructorWithDefaultThreads)
{
    ThreadPool pool;
    EXPECT_GT(pool.size(), 0);
    EXPECT_LE(pool.size(), std::thread::hardware_concurrency());
}

TEST_F(ThreadPoolTest, ConstructorWithSpecificThreads)
{
    ThreadPool pool(4);
    EXPECT_EQ(pool.size(), 4);
}

TEST_F(ThreadPoolTest, ConstructorWithZeroThreads)
{
    ThreadPool pool(0);
    EXPECT_EQ(pool.size(), 1);  // Should default to 1 thread
}

TEST_F(ThreadPoolTest, SubmitSimpleTask)
{
    ThreadPool pool(2);
    auto future = pool.submit([]() { return 42; });
    EXPECT_EQ(future.get(), 42);
}

TEST_F(ThreadPoolTest, SubmitTaskWithArguments)
{
    ThreadPool pool(2);
    auto future = pool.submit([](int a, int b) { return a + b; }, 10, 20);
    EXPECT_EQ(future.get(), 30);
}

TEST_F(ThreadPoolTest, SubmitMultipleTasks)
{
    ThreadPool pool(4);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 10; ++i)
    {
        futures.push_back(pool.submit([i]() { return i * i; }));
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(futures[i].get(), i * i);
    }
}

TEST_F(ThreadPoolTest, ParallelExecution)
{
    ThreadPool pool(4);
    std::atomic<int> counter(0);

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 20; ++i)
    {
        futures.push_back(pool.submit(
            [&counter]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                counter++;
            }));
    }

    for (auto& future : futures)
    {
        future.wait();
    }

    EXPECT_EQ(counter, 20);
}

TEST_F(ThreadPoolTest, WaitForCompletion)
{
    ThreadPool pool(2);
    std::atomic<int> counter(0);

    for (int i = 0; i < 10; ++i)
    {
        pool.submit(
            [&counter]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                counter++;
            });
    }

    pool.wait();
    EXPECT_EQ(counter, 10);
}

TEST_F(ThreadPoolTest, PendingTasksCount)
{
    ThreadPool pool(1);  // Only 1 thread to create backlog

    // Submit tasks that take time
    for (int i = 0; i < 5; ++i)
    {
        pool.submit(
            []()
            { std::this_thread::sleep_for(std::chrono::milliseconds(50)); });
    }

    // Should have pending tasks
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_GT(pool.pendingTasks(), 0);
}

TEST_F(ThreadPoolTest, ExceptionHandling)
{
    ThreadPool pool(2);
    auto future = pool.submit(
        []() -> int
        {
            throw std::runtime_error("Test exception");
            return 42;
        });

    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(ThreadPoolTest, ThreadSafety)
{
    ThreadPool pool(4);
    std::atomic<int> counter(0);
    std::mutex mtx;
    std::vector<int> results;

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 100; ++i)
    {
        futures.push_back(pool.submit(
            [&counter, &mtx, &results, i]()
            {
                int value = counter++;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    results.push_back(value);
                }
            }));
    }

    for (auto& future : futures)
    {
        future.wait();
    }

    EXPECT_EQ(counter, 100);
    EXPECT_EQ(results.size(), 100);
}

TEST_F(ThreadPoolTest, DestructorWaitsForTasks)
{
    std::atomic<int> counter(0);

    {
        ThreadPool pool(2);
        for (int i = 0; i < 10; ++i)
        {
            pool.submit(
                [&counter]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    counter++;
                });
        }
        // Destructor should wait for all tasks
    }

    EXPECT_EQ(counter, 10);
}

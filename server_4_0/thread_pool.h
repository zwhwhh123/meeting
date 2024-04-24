
/*
 * @file        thread_poll.h
 * @brief       Brief description of the class
 * @author      nhmt
 * @date        2024/03/25
 */

#ifndef THREAD_POLL_H
#define THREAD_POLL_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <cassert>

class ThreadPool {
private:
    // 内部结构体，用于存储线程池的相关信息
    struct Pool {
        // 互斥量，用于保护临界资源的访问
        std::mutex mtx;
        // 条件变量，用于线程之间的通信和同步
        std::condition_variable cond;
        // 表示线程池是否已关闭的标志
        bool isClosed = false;
        // 任务队列，存储需要执行的任务
        std::queue<std::function<void()>> tasks;
    };
    // 线程池共享的内部结构体指针
    std::shared_ptr<Pool> pool_;
    // 存储工作线程的容器
    std::vector<std::thread> workers;
public:
    // 构造函数，创建线程池
    explicit ThreadPool(size_t threadCount = 8) : pool_(std::make_shared<Pool>()) {
        assert(threadCount > 0);
        // 根据指定的线程数量创建工作线程
        for (size_t i = 0; i < threadCount; i++) {
            workers.emplace_back([pool = pool_] {
                while (true) {
                    // 上锁保护临界资源
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    // 等待条件满足或线程池关闭
                    pool->cond.wait(locker, [pool] {
                        return pool->isClosed || !pool->tasks.empty();
                        });
                    // 如果线程池关闭且任务队列为空，线程退出
                    if (pool->isClosed && pool->tasks.empty()) return;
                    // 获取并执行任务
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    locker.unlock();
                    task();
                }
                });
        }
    }
    // 析构函数，关闭线程池
    ~ThreadPool() {
        // 如果线程池仍然存在
        if (static_cast<bool>(pool_)) {
            {
                // 上锁修改关闭标志
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            // 唤醒所有线程，使其退出循环
            pool_->cond.notify_all();
        }
        // 等待所有工作线程结束
        for (std::thread& worker : workers)
            worker.join();
    }
    // 向任务队列添加任务并返回一个与任务关联的 future 对象
    template <class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        // 创建 packaged_task，将任务绑定到函数对象上
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        // 获取与任务关联的 future 对象
        std::future<return_type> res = task->get_future();
        {
            // 上锁保护任务队列
            std::lock_guard<std::mutex> locker(pool_->mtx);
            // 如果线程池已关闭，则抛出异常
            if (pool_->isClosed) {
                throw std::runtime_error("Enqueue on closed ThreadPool");
            }
            // 将任务加入任务队列
            pool_->tasks.emplace([task]() { (*task)(); });
        }
        // 唤醒一个等待中的线程来执行任务
        pool_->cond.notify_one();
        return res;
    }

};

#endif /* THREAD_POLL_H */


// int main() {
//     // Create a thread pool with 8 threads
//     ThreadPool pool(8);
//     // Enqueue tasks
//     std::vector<std::future<void>> futures;
//     for (int i = 0; i < 10; ++i) {
//         futures.emplace_back(pool.Enqueue([i] {
//             std::this_thread::sleep_for(std::chrono::seconds(1));
//             std::cout << "Task " << i << " completed" << std::endl;
//             }));
//     }
//     // Wait for all tasks to complete
//     for (auto& future : futures) {
//         future.wait();
//     }
//     return 0;
// }

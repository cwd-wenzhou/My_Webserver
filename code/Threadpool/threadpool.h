#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <assert.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    //explicit声明显示类构造函数
    //make_shared 创建Pool以及其的智能指针
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {

            assert(threadCount > 0);//断言一下threadCount参数>0，否则直接返回异常
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {
                    std::unique_lock<std::mutex> locker(pool->mtx);//unique_lock的构造函数默认直接锁上互斥量
                    while(true) {
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());//用移动语义把任务队列里的任务函数交给线程。
                            pool->tasks.pop();//从任务队列里删除已交给线程的函数
                            locker.unlock();
                            task(); //仅执行任务的时候不用上锁
                            locker.lock();
                        } 
                        else if(pool->isClosed) break; //用于关闭线程
                        else pool->cond.wait(locker);//阻塞线程，等待任务队列里有新的任务才被唤醒
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        //若pool_非空再进来析构，按逻辑是肯定会进到这个if里面，但是还是加一个if保护一下，以免报错
        //static_cast就是把pool_强转成bool
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();//通知所有线程关闭
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();//增加任务后，唤醒一个线程就够了
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;//条件变量
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H
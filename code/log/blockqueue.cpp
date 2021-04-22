/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#include "blockqueue.h"

template<class T>
BlockDeque<T>::BlockDeque(size_t Capacity = 1000):capacity_(capacity)
{
        assert(Capacity>0);
        isClose_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque()
{
        Close();
}

template<class  T>
void BlockDeque<T>::clear(){
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();
}

template<class  T>
bool BlockDeque<T>::empty(){
        std::lock_guard<std::mutex> locker(mtx_);
        return deq_.empty();
}

template<class  T>
bool BlockDeque<T>::full(){
        std::lock_guard<std::mutex> locker(mtx_);
        return deq_.size()>=capacity;
}

template<class  T>
void BlockDeque<T>::Close(){
        {
                std::lock_guard<std::mutex> locker(mtx_);
                deq_.clear();
                isClose_=true;
        }
        //isClose_置成true后，通知所有
        condConsumer_.notify_all();
        condProducer_.notify_all();
}

template<class  T>
size_t BlockDeque<T>::size(){
        std::lock_guard<std::mutex> locker(mtx_);
        return deq_.size();
}

template<class  T>
size_t BlockDeque<T>::capacity(){
        std::lock_guard<std::mutex> locker(mtx_);
        return capacity_;
}

template<class  T>
T BlockDeque<T>::front(){
        std::lock_guard<std::mutex> locker(mtx_);
        return deq_.front();
}

template<class  T>
T BlockDeque<T>::back(){
        std::lock_guard<std::mutex> locker(mtx_);
        return deq_.back();
}

template<class  T>
void BlockDeque<T>::push_front(const T&item){
        std::lock_guard<std::mutex> locker(mtx_);
        while (deq_.size()>=capacity_){
                //若队列满，等待生产者信号量
                if (condProducer_.wait(locker)
                        return false;//因为超时，返回false
                if (isClose_) return；//因为队列已经关闭，直接返回
        }
        deq_.push_front(item);
        condConsumer_.notify_one();
}

template<class  T>
void BlockDeque<T>::push_back(const T&item){
        std::lock_guard<std::mutex> locker(mtx_);
        while (deq_.size()>=capacity_){
                //若队列满，等待生产者信号量
                if (condProducer_.wait(locker)
                        return false;//因为超时，返回false
                if (isClose_) return；//因为队列已经关闭，直接返回
        }
        deq_.push_back(item);
        condConsumer_.notify_one();
}

template<class  T>
bool BlockDeque<T>::pop(T&item){
        std::lock_guard<std::mutex> locker(mtx_);
        while (deq_.empty())
        {
                //若队列为空，等待消费者信号量
                //若已经关闭，直接返回false
                condConsumer_.wait(locker);
                if (isClose_)
                        return false;
        }
        item=deq_.front();
        deq_.pop_front();
        condProducer_.notify_one();
        return true;
}

template<class  T>
bool BlockDeque<T>::pop(T&item,int timeout){
        std::lock_guard<std::mutex> locker(mtx_);
        while (deq_.empty())
        {
                //若队列为空，等待消费者信号量
                if (condConsumer_.wait_for(locker,std::chrono::seconds(timeout))==std::cv_status::timeout)
                        return false;//因为超时，返回false
                if (isClose_)
                        return false;//因为队列已经关闭，直接返回false
        }
        item=deq_.front();
        deq_.pop_front();
        condProducer_.notify_one();
        return true;
}
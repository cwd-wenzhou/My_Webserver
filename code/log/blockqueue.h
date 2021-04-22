/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <assert.h>
#include <deque>
#include <mutex>
#include <condition_variable>

template<class T>
class BlockDeque
{
private:
        std::deque<T> deq_;

        size_t capacity_;

        std::mutex mtx_;

        bool isClose_;

        std::condition_variable condConsumer_;

        std::condition_variable condProducer_;
public:
        explicit BlockDeque(size_t Capacity = 1000);

        ~BlockDeque();

        void clear();

        bool empty();

        bool full();

        void Close();

        size_t size();

        size_t capacity();

        T front();

        T back();

        void push_back(const T&item);

        void push_front(const T&item);

        bool pop(T&item);

        bool pop(T&item,int timeout);
};

#endif
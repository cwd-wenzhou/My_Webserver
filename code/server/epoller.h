/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#ifndef EPOLLER_H
#define EPOLLER_H

#include <assert.h>
#include <vector>
#include <sys/epoll.h>

class Epoller
{
private:
        int epollFd_;  //epoll_create句柄
        std::vector<struct epoll_event> events_;//作为epoll_wait的返回值
public:
        //explicit 关键字声明，阻止Max_Event_Num的隐式类型转换
        explicit Epoller(int Max_Event_Num=1024);

        ~Epoller();

        bool AddFd(int fd,uint32_t events);//增加事件

        bool ModFd(int fd,uint32_t events);//修改事件

        bool DelFd(int fd);//删除事件

        int Wait(int timeout_Ms=-1);//调用epollwait

        int GetEventFd(size_t i) const;//const成员函数中不允许对数据成员进行修改

        uint32_t GetEvents(size_t i) const;
};

#endif //EPOLLER_H
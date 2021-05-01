/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#include "epoller.h"
Epoller::Epoller(int Max_Event_Num):epollFd_(epoll_create(512)),events_(Max_Event_Num)
{
        assert(epollFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller()
{
}

bool Epoller::AddFd(int fd,uint32_t events){
        if (fd<0) return false;
        epoll_event ev={0};
        ev.data.fd=fd;
        ev.events=events;
        return (0==epoll_ctl(epollFd_,EPOLL_CTL_ADD,fd,&ev));
}

bool Epoller::ModFd(int fd,uint32_t events){
        if (fd<0) return false;
        epoll_event ev={0};
        ev.data.fd=fd;
        ev.events=events;
        return (0==epoll_ctl(epollFd_,EPOLL_CTL_MOD,fd,&ev));
}

bool Epoller::DelFd(int fd){
        if (fd<0) return false;
        epoll_event ev={0};
        return (0==epoll_ctl(epollFd_,EPOLL_CTL_DEL,fd,&ev));
}

int Epoller::Wait(int timeout_Ms){
        return epoll_wait(epollFd_,&events_[0],(int)(events_.size()),timeout_Ms);
        //static_cast<int>(events_.size())   后面不是表达式，直接强转了算了
}

int Epoller::GetEventFd(size_t i)const {
        assert(i<events_.size() && i>=0);
        return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i)const {
        assert(i<events_.size() && i>=0);
        return events_[i].events;
}
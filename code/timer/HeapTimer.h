/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#ifndef HEAP_TIMER_
#define HEAP_TIMER_

#include <queue>
#include <unordered_map>
#include <time.h>
#include <assert.h>
#include <functional>
#include <chrono>

typedef  std::function<void()> Timeout_Callback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
        int id;
        TimeStamp expires;
        Timeout_Callback cb;
        bool operator<(const TimerNode  &t) const{
                return expires<t.expires;
        }
        bool operator>(const TimerNode  &t) const{
                return expires>t.expires;
        }
};
class HeapTimer
{
private:
        std::vector<TimerNode> heap_;

        std::unordered_map<int,size_t> hash;//<TimerNode.id,该id的TimerNode在heap_中的位置>

        void Swap_Nodes(size_t i,size_t j);      

        void Shift_Up(size_t i);

        void Shift_Down(size_t i);

        void Del_Node(size_t i);

public:
        HeapTimer(/* args */);
        ~HeapTimer();

        void Add_Node(int id,int timeout,const Timeout_Callback &cb);

        void Adjust_Node(int id,int timeout);

        void Pop();
        
        void Run_Callback(int id);

        void tick();//运行并清除超时节点

        int Get_Next_Tick();
};


#endif
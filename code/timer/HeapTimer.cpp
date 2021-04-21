/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#include "HeapTimer.h"
HeapTimer::HeapTimer(/* args */)
{
        heap_.reserve(64);//先预留一点空间
}

HeapTimer::~HeapTimer()
{
        hash.clear();
        heap_.clear();
}

void HeapTimer::Swap_Nodes(size_t i,size_t j){
        assert(i>0 && i<heap_.size());
        assert(j>0 && j<heap_.size());
        
        //swap交换。看了一眼c++11的实现，用了模板和移动语义，很快，不需要自己写；
        std::swap(heap_[i],heap_[j]);

        //更新hash表
        hash[heap_[i].id]=i;
        hash[heap_[j].id]=j;
}

//把vector写成完全二叉树，父节点id=(子节点id-1)/2；

//将i节点一路向上作交换，直到到它该在的位置
void HeapTimer::Shift_Up(size_t i){
        assert(i>0 && i<heap_.size());
        int j=(i-1)/2;
        while (i>0){
                if (heap_[j]<heap_[i])  break;
                Swap_Nodes(i,j);
                i=j;
                j=(i-1)/2;
        }
}

//将i节点一路向下作交换，直到到它该在的位置
void HeapTimer::Shift_Down(size_t i){
        assert(i>=0 && i<heap_.size());
        int j=i*2+1;
        int N=heap_.size();
        while (j<N){
                if (j+1<N && heap_[j+1]<heap_[j]) j++;
                if (heap_[i]<heap_[j]) break;
                Swap_Nodes(i,j);
                i=j;
                j=i*2+1;
        }
}
//1：将要删除的节点换到最后一个；
//2：删掉最后一个节点；
//3：把换上去的节点下沉
void HeapTimer::Del_Node(size_t i){
        assert(i>=0 && i<heap_.size());
        int N=heap_.size();
        //若i本就在最后一个，删掉就好了。
        if (i==N-1){
                hash.erase(heap_.back().id);
                heap_.pop_back();
                return;
        }
        Swap_Nodes(i,N-1);
        hash.erase(heap_.back().id);
        heap_.pop_back();
        Shift_Down(i);
}

void HeapTimer::Add_Node(int id,int timeout,const Timeout_Callback &cb){
        size_t i;
        if (hash.count(id)>0){
                //已有结点
                i=hash[id];
                heap_[i].cb=cb;
                heap_[i].expires=Clock::now()+MS(timeout);
                Shift_Down(i);
                Shift_Up(i);
        }
        else{
                //创建新节点
                i=heap_.size();
                hash[id]=i;
                heap_.push_back({id,Clock::now() + MS(timeout), cb});
                Shift_Up(i);
        }
}

void HeapTimer::Adjust_Node(int id,int timeout){
        assert(!heap_.empty() && hash.count(id)>0);
        heap_[hash[id]].expires=Clock::now()+MS(timeout);
        Shift_Down(hash[id]);//认为调整只会增加时间。
        //Shift_Up(hash[id]);
}

void HeapTimer::Pop(){
        assert(!heap_.empty());
        Del_Node(0);
}

void HeapTimer::Run_Callback(int id){
        //if (heap_.empty() || hash.count(id)==0) return ;
        assert(!heap_.empty() && hash.count(id)>0);
        size_t i=hash[id];
        //heap_[i].cb();
        TimerNode node = heap_[i];
        node.cb();//Run_Callback 
        Del_Node(i);
}

void HeapTimer::tick(){
        if (heap_.empty()){
                return;
        }
        while (!heap_.empty()){
                TimerNode node = heap_.front();
                if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
                        break; 
                }
                node.cb();
                Pop();
        }
}

int HeapTimer::Get_Next_Tick(){
        tick();
        size_t res = -1;
        if(!heap_.empty()) {
                res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
                if(res < 0) { res = 0; }
        }
        return res;
}
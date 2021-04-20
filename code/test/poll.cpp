#include<poll.h>
#include<signal.h>
#include<iostream>
  #include <unistd.h>
using namespace std;
int main(){
        /*第一步 poll开始监听之前要知道要监管哪些套接字*/
            struct pollfd fds[1];
            fds[0].fd=0;
            fds[0].events=POLL_IN;
            fds[0].revents=POLL_IN;
            
        /*第二步 poll开始工作 阻塞的轮询看监管的套接字是否就绪*/

          int ret=poll(fds,1,5000);

        /*第三部  poll完成工作  有套接字就绪或者时间超时返回*/
        if(ret<0){cout<<"error"<<endl;}
        else if(ret==0){cout<<"time out"<<endl;}
        else{
            if(fds[0].revents==POLL_IN)
                {
                    char message[10];
                    read(fds[0].fd,message,sizeof(message));
                    cout<<message<<endl;
                }
    
        }
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include<strings.h>
#include<sys/socket.h>
#include<iostream>
#include<arpa/inet.h>
using namespace std;

int main(void) {
    
    /**step1 :  select工作之前,需要知道要监管哪些套接字**/
   int listen_fd1=0;
   int listen_fd2=1;
    fd_set  read_set;   
    FD_ZERO(&read_set);
    FD_SET(listen_fd1,&read_set);
    FD_SET(listen_fd2,&read_set);

    /*step2 : select开始工作,设定时间内阻塞轮询套接字是否就绪*/
    struct timeval tv;
       tv.tv_sec = 5;
        tv.tv_usec = 0;
    int ret=select(listen_fd2+1,&read_set,NULL,NULL,&tv);

    /*step3 : select完成工作,即如果出现就绪或者超时 ,则返回*/
    if(ret==-1){
        cout<<"errno!"<<endl;
    }
    else if(ret==0){
        cout<<"time out"<<endl;
    }
    else if(ret>0){
            if(FD_ISSET(listen_fd1,&read_set));
             {  
                 char *buffer=new char[100];
                 read(listen_fd1,buffer,sizeof(&buffer));
                 cout<<"Input String ********** : "<<buffer<<endl;
             }

             if(FD_ISSET(listen_fd2,&read_set));
             {  
                 char *buffer=new char[100];
                 read(listen_fd2,buffer,sizeof(buffer));
                 cout<<"Input String : "<<buffer<<endl;
             }

    }

}
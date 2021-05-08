#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include<stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1023

int setnoblocking(int fd){
    int old_option = fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,old_option | O_NONBLOCK);
    return old_option;
}

void addfd(int epollfd,int fd,bool enable_et){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et)
        event.events |= EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnoblocking(fd);
}



/*
man connect 里面有这样的一段话
       EINPROGRESS
              The  socket  is  nonblocking  and  the connection cannot be completed immediately.  It is possible to select(2) or poll(2) for completion by
              selecting the socket for writing.  After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level  SOL_SOCKET
              to  determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed
              here, explaining the reason for the failure).
*/
//非阻塞connect
int unblock_connect(const char* ip,int port,int time){
    int ret=0;
    struct  sockaddr_in address;
    bzero(&address,sizeof(address));

    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);

    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd>0);

    int fdold_opt=setnoblocking(sockfd);

    ret = connect(sockfd,(struct sockaddr*)&address,sizeof(address));
    if (ret == 0){
        //连接成功，恢复socket属性并返回
        printf("connect with server successed\n");
        fcntl(sockfd,F_SETFL,fdold_opt);
        return sockfd;
    }
    else if (errno !=   EINPROGRESS){
            printf("unblock connect not support\n");
            return -1;
    }
    fd_set readfds;
    fd_set writefds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds);
    timeout.tv_sec = time;
    timeout.tv_usec =0;

    ret = select(sockfd+1,NULL,&writefds,NULL,&timeout);
    if (ret<0){
        //select出错或者超时
        printf("overtime or select error!\n");
        close(sockfd);
        return -1;
    }

    if ( !FD_ISSET(sockfd,&writefds)){
        //没有读就绪事件
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }
    int error = 0;
    socklen_t length = sizeof(error);
    if (getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&length) < 0){
        printf("GET SOCKET OPTION FAILED\n ");
        close(sockfd);
        return -1;
    }
    if (error != 0){
        printf("connection failed after select with the error:%d\n",error);
        close(sockfd);
        return -1;
    }
    printf("connection ready after select with the socket:%d\n",sockfd);
    fcntl(sockfd,F_SETFL,fdold_opt);
    return sockfd;
}


int main(int argc, char const *argv[])
{
    if (argc <=2)
    {
        printf("usags ip_addreee port_number\n");
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = unblock_connect(ip,port,10);
    if (sockfd<0){
        return 1;
    }
    close(sockfd);
    return 0;
}

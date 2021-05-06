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
#define BUFFER_SIZE 10

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

void lt(epoll_event* events,int number,int epollfd,int listenfd){
    char buf[BUFFER_SIZE];
    for (int i=0;i<number;i++){
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd){
            struct sockaddr_in client_address;
            socklen_t client_length=sizeof(client_address);
            int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_length);
            addfd(epollfd,connfd,false);
        }
        else if (events[i].events & EPOLLIN){
            printf("event trigger once\n");
            memset(buf,'\0',BUFFER_SIZE);
            int ret = recv (sockfd,buf,BUFFER_SIZE-1,0);
            if (ret<=0){
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content:%s\n",ret,buf);
        }
        else  
            printf("something else happened\n");
    }
}

void et(epoll_event* events,int number,int epollfd,int listenfd){
    char buf[BUFFER_SIZE];
    for (int i=0;i<number;i++){
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd){
            struct sockaddr_in client_address;
            socklen_t client_length=sizeof(client_address);
            int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_length);
            addfd(epollfd,connfd,true);
        }
        else if (events[i].events & EPOLLIN){
            printf("event trigger once\n");
            while (1){
                memset(buf,'\0',BUFFER_SIZE);
                int ret = recv (sockfd,buf,BUFFER_SIZE-1,0);
                if (ret<0){
                    if ((errno==EAGAIN) || (errno == EWOULDBLOCK)){
                        printf("READ LATER\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if (ret==0){
                    close(sockfd);
                }
                else 
                    printf("get %d bytes of content:%s\n",ret,buf);
            }
        }
        else  
            printf("something else happened\n");
    }
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

    int ret=0;
    struct  sockaddr_in address;
    bzero(&address,sizeof(address));

    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);

    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd>0);

    ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret = listen(listenfd,5);
    assert(ret!=-1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd!=-1);
    addfd(epollfd,listenfd,true);

    while (1){
        int ret =epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        assert(ret>=0);
        //lt(events,ret,epollfd,listenfd);
        et(events,ret,epollfd,listenfd);
    }
    close(listenfd);
    return 0;
}

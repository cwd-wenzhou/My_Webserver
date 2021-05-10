#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include<stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string.h>

#define BUFFER_SIZE 64
int main(int argc, char const *argv[])
{
    if (argc <=2)
    {
        printf("usags ip_addreee port_number\n");
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(port);
    inet_pton(AF_INET,ip,&server_address.sin_addr);

    int sockfd = socket(PF_INET,SOCK_STREAM,0);//创建套接字
    assert(sockfd>=0);

    if (connect(sockfd,(struct sockaddr*)&server_address,sizeof(server_address))<0){
        //连接套接字，若失败进到这个if里面处理。
        printf("connect error\n");
        close(sockfd);
        return 1;
    }
    printf("connect to server successfully\n");
    //注册标准输入(文件描述符为0),以及sockfd上的可读事件
    pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN |  POLLRDHUP;
    fds[1].revents = 0;

    char read_buff[BUFFER_SIZE];//读内容的 用户缓存区
    //创建管道
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret!=-1);

    while (1){
        ret = poll(fds,2,-1);
        if (ret<0){
            printf("poll failed\n");
            break;
        }
        if (fds[1].revents & POLLRDHUP){
            printf("SERVER COLSED\n");
            break;
        }

        if (fds[1].revents & POLLIN){
            //server给客户端发了信息，读进来，从控制台输出出去
            memset(read_buff,'\0',BUFFER_SIZE);
            recv(fds[1].fd,read_buff,BUFFER_SIZE-1,0);
            printf("%s\n",read_buff);
        }

        if (fds[0].revents & POLLIN){
            ret = splice(0,NULL,pipefd[1],NULL,32768,SPLICE_F_MORE |SPLICE_F_MOVE);
            assert(ret!=-1);
            ret = splice(pipefd[0],NULL,sockfd,NULL,32768,SPLICE_F_MORE |SPLICE_F_MOVE);
            assert(ret!=-1);
            //printf("write\n");
        }
    }
    close(sockfd);
    return 0;
}

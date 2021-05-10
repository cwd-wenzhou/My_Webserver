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
#include <errno.h>

#define BUFFER_SIZE 64
#define USER_LIMIT 5
#define FD_LIMIT 65535

int setnoblocking(int fd){
    int old_option = fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,old_option | O_NONBLOCK);
    return old_option;
}

/*
客户数据：
客户端socket地址
待写到客户端的数据
从客户端读入的数据
*/
struct client_data
{
    sockaddr_in address;
    char * write_buf;
    char buf[BUFFER_SIZE];
};

int main(int argc, char const *argv[])
{
    if (argc <=2)
    {
        printf("usags ip_addreee port_number\n");
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr);

    int listenfd = socket(PF_INET,SOCK_STREAM,0);//创建套接字
    assert(listenfd>=0);

    int ret=bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret=listen(listenfd,5);//backlog就设成5
    assert(ret!=-1);

    //直接建hash，每个fd对应的client_data，直接通过下标检索
    client_data* users = new client_data[FD_LIMIT];

    pollfd fds[USER_LIMIT+1];
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    for  (int i=1;i<=USER_LIMIT;i++){
        fds[i].events = POLLIN | POLLERR;
        fds[i].fd = -1;
    }

    int usercount  = 0;
    while (1)
    {
        ret = poll(fds,usercount+1,-1);
        if (ret<0){
            printf("poll failed\n");
            break;
        }

        //新的客户端连进来了
        if (fds[0].revents & POLLIN){
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
            if (connfd<0){
                printf("errno is :%d\n",errno);
            }
            //请求超过用户总数
            if (usercount>= USER_LIMIT){
                const char* info = "too many users,pls wait\n";
                printf("%s",info);
                send(connfd,info,strlen(info),0);
                continue;
            }
            setnoblocking(connfd);
            usercount++;
            fds[usercount].events = POLLIN |    POLLRDHUP  | POLLERR ;
            fds[usercount].fd = connfd;
            fds[usercount].revents = 0;
            users[connfd].address = client_address;
            printf("comes new user,there are %d users\n",usercount);
        }

        for (int i=1;i<=usercount;i++){
            //printf("fds[%d].events=%d\n",i,fds[i].events);
            if (fds[i].revents & POLLERR){
                //出现错误，调一下getsockopt获取错误信息
                printf("get error  in fd=%d\n",fds[i].fd);
                char errinfo[100];
                memset(errinfo,'\0',100);
                socklen_t len = sizeof(errinfo);
                if (getsockopt(fds[i].fd,SOL_SOCKET,SO_ERROR,&errinfo,&len)<0)
                    printf("get socket error error\n");
                else
                    printf("err_info=%s\n",errinfo);
                continue;
            }
            else if (fds[i].revents & POLLRDHUP){
                //客户端关闭连接
                users[fds[i].fd] = {0};
                //users[fds[i].fd] = users[fds[usercount].fd];
                close(fds[i].fd);
                fds[i]=fds[usercount];
                i--;
                usercount--;
                printf("a client left\n");
            }
            else if (fds[i].revents & POLLOUT){
                //socket 写就绪
                int connfd = fds[i].fd;
                if (! users[connfd].write_buf)
                    continue;//写入缓存中没有东西，则直接跳过
                send(connfd,users[connfd].write_buf,strlen(users[connfd].write_buf),0);
                users[connfd].write_buf = nullptr;
                fds[i].events &=~POLLOUT;
                //printf("fds[i].events=%d\n",fds[i].events);
                fds[i].events |= POLLIN;
            }
            else if (fds[i].revents & POLLIN){
                //socket读就绪
                int connfd = fds[i].fd;
                memset(users[connfd].buf,'\0',BUFFER_SIZE);//清空缓存
                ret = recv(connfd,users[connfd].buf, BUFFER_SIZE-1,0);
                if (ret<0){
                    printf("read error then kick this fd(%d) out\n",connfd);
                    users[fds[i].fd] = {0};
                    //users[fds[i].fd] = users[fds[usercount].fd];
                    close(fds[i].fd);
                    fds[i]=fds[usercount];
                    i--;
                    usercount--;
                }
                else if (ret==0)
                {
                    //读进来是空的
                    //printf("read a empty message\n");
                }
                else{
                    //收到有内容的数据，通知其他所有socket准备写
                    for (int j=1;j<=usercount;j++){
                        //printf("fds[%d].events=%d\n",j,fds[j].events);
                        if (fds[j].fd == connfd)
                            continue;
                        fds[j].events &= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        //printf("now fds[%d].events=%d\n",j,fds[j].events);
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                    printf("GET %d bytes of client data: %s from %d\n",ret,users[connfd].buf,connfd);
                }
            }
        }
    }
    delete [] users;
    close(listenfd);
    return 0;
}

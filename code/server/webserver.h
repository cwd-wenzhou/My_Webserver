/*
 * @Author       : cwd
 * @Date         : 2021-4-27
 * @Place  : hust
 * 知识点：
 *              1：bind 绑定的函数是成员函数的时候，后面跟的第一个要是this
 */ 
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <unistd.h> //getcwd()
#include <assert.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/HeapTimer.h"
#include "../sql/Sql_Connect_Pool.h"
#include "../sql/Sql_Connect_RAII.h"
#include "../http/httpconn.h"
#include "../Threadpool/threadpool.h"

class WebServer
{
public:
        WebServer(
                int port,int trigMode,int timeoutMS,bool OptLinger,
                int sqlPort, const char* sqlUser, const  char* sqlPwd, 
                const char* dbName, int connPoolNum, int threadNum,
                bool openLog, int logLevel, int logQueSize
        );
        ~WebServer();
        void Start();
private:

        void InitEventMode_(int trigMode);
        bool InitSocket_();
        void AddClient_(int fd,sockaddr_in addr);

        void SendError_(int fd, const char*info);
        void ExtentTime_(HttpConn* client);
        void CloseConn_(HttpConn* client);
        
        void DealListen_();
        void DealWrite_(HttpConn* client);
        void DealRead_(HttpConn* client);

        void OnRead_(HttpConn* client);
        void OnWrite_(HttpConn* client);
        void OnProcess(HttpConn* client);

        static const int MAX_FD = 65535;
        static int SetFdNonblock(int fd);

        int port_;
        int openLinger_;
        int trigMode_;
        int timeoutMS_;
        bool isClose_;
        int listenFd_;
        char *srcDir_;

        uint32_t listenEvent_;
        uint32_t connEvent_;
   
        std::unique_ptr<HeapTimer> timer_;
        std::unique_ptr<ThreadPool> threadpool_;
        std::unique_ptr<Epoller> epoller_;
        std::unordered_map<int, HttpConn> users_;
};


#endif //WEBSERVER_H

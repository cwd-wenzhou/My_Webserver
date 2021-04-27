/*
 * @Author       : cwd
 * @Date         : 2021-04-26
 * @Place      :hust
 * 
 * 知识点:
 * 
 */ 
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>   // sockaddr_in
#include <assert.h>

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../sql/Sql_Connect_RAII.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn
{
private:
        Buffer readBuff_;  //读缓存区
        Buffer writeBuff_;//写缓存区

        int fd_;//该连接的文件描述符
        struct sockaddr_in addr_;
        bool isClose_;

        int iovCnt_;
        struct iovec iov_[2];

        HttpRequest request_;
        HttpResponse response_;
public:
        HttpConn(/* args */);
        ~HttpConn();

        static const char* srcDir;//资源目录路径
        static std::atomic<int> userCount;//http连接数计数
        static bool isET;//使用ET还是LT

        const char* GetIP() const;
        const int GetPort() const;
        int GetFd() const;
        bool IsKeepAlive() const;
        sockaddr_in GetAddr() const;

        void Init(int fd, const sockaddr_in &addr);
        void Close();
        ssize_t read(int* SaveErrno);
        ssize_t write(int* SaveErrno);

        bool process();
};




#endif //HTTP_CONN_H
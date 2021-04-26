/*
 * @Author       : cwd
 * @Date         : 2021-04-26
 * @Place      :hust
 * 
 * 知识点:
 * 
 */ 

#include "httpconn.h"

HttpConn::HttpConn(/* args */)
{
        fd_=-1;
        addr_={0};
        isClose_=true;
}

HttpConn::~HttpConn()
{

}

const char* HttpConn::GetIP() const{
        return inet_ntoa(addr_.sin_addr);
}

const int HttpConn::GetPort() const{
        return (addr_.sin_port);
}

int HttpConn::GetFd() const{
        return (fd_);
}

bool HttpConn::IsKeepAlive() const {
        return request_.IsKeepAlive();
}

sockaddr_in HttpConn::GetAddr() const{
        return addr_;
}

void HttpConn::Init(int fd, const sockaddr_in &addr){
        assert(fd>0);
        fd_=fd;
        addr_=addr;
        userCount++;//计数
        readBuff_.RetrieveAll();//清空读缓存区
        writeBuff_.RetrieveAll();//清空写缓存区
        isClose_=false;
        Log_Info("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

ssize_t HttpConn::read(int* SaveErrno){
        ssize_t len=-1;
        do {
                len = readBuff_.ReadFd(fd_,SaveErrno);
                if (len<0) break;
        }while (isET);
        return len;
}

ssize_t HttpConn::write(int* SaveErrno){
        size_t len=-1;
        do{
                len = writev(fd_,iov_,iovCnt_);
                if (len<=0){
                        *SaveErrno=errno;
                        break;
                }
                if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
                else if (static_cast<size_t>(len)>iov_[0].iov_len){
                        iov_[1].iov_base = (uint8_t*) iov_[1].iov_base +(len-iov_[0].iov_len);
                        iov_[1].iov_len-=(len - iov_[0].iov_len);
                        if (iov_[0].iov_len){
                                writeBuff_.RetrieveAll();
                                iov_[0].iov_len=0;
                        }
                }
                else {
                        iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
                        iov_[0].iov_len -= len; 
                        writeBuff_.Retrieve(len);
                }
        }while (isET || (iov_[0].iov_len + iov_[1].iov_len>len));
        return len;
}
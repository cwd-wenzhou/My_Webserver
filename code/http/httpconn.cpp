/*
 * @Author       : cwd
 * @Date         : 2021-04-26
 * @Place      :hust
 * 
 * 知识点:
 * 
 */ 

#include "httpconn.h"

HttpConn::HttpConn(/* args */){
        fd_=-1;
        addr_={0};
        isClose_=true;
}

HttpConn::~HttpConn(){
        Close();
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
        }while (isET || (ToWriteBytes()>10240));
        return len;
}

void HttpConn::Close(){
        response_.UnmapFile();//解除文件映射
        if (!isClose_){
                isClose_=true;
                userCount--;
                close(fd_);
                Log_Info("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
        }
}

bool HttpConn::process(){
        request_.Init();
        if (readBuff_.Readable_Bytes()<=0) return false;
        else if (request_.parse(readBuff_)){
                Log_Debug("%s",request_.path().c_str());
                response_.Init(srcDir,request_.path(),request_.IsKeepAlive(),200);
        }
        else{
                response_.Init(srcDir,request_.path(),false,400);//请求报文已经有错了，返回一个响应报文就行了
        }

        response_.MakeResponse(writeBuff_);//向writeBuff_写入响应报文
        
        //响应头
        iov_[0].iov_base  = const_cast<char*>(writeBuff_.ReadPos_Ptr_());
        iov_[0].iov_len = writeBuff_.Readable_Bytes();
        iovCnt_=1;

        //文件
        if (response_.FileLen()>0 && response_.File()){
                iov_[1].iov_base = response_.File();
                iov_[1].iov_len = response_.FileLen();
                iovCnt_ = 2;
        }
        Log_Debug("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, iov_[0].iov_len + iov_[1].iov_len);
        return true;
}
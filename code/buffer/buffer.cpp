/*
 * @Author       : cwd
 * @Date         : 2021-4-23
 * @Place  : hust
 */ 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

Buffer::~Buffer()
{
}

const char * Buffer::BeginPtr() const{
        return &*buffer_.begin();
}

char * Buffer::BeginPtr() {
        return &*buffer_.begin();
}

void Buffer::MakeSpace(size_t len){
        if (Writeable_Bytes()+readPos_<len){
                buffer_.resize(writePos_+len+1);
                //若buff不够，直接申请更多空间
        }
        else{
                size_t readable=Readable_Bytes();
                std::copy(ReadPos_Ptr_(),WritePos_Ptr_const_(),BeginPtr());
                readPos_=0;
                writePos_=readable;
                assert(readable == Readable_Bytes());//个人认为是多余的代码
        }
}

size_t Buffer::Writeable_Bytes() const {
        return buffer_.size() - writePos_;
}

size_t Buffer::Readable_Bytes() const {
        return writePos_ - readPos_;
}

size_t Buffer::Prependable_Bytes() const {
        return readPos_;
}

const char* Buffer::ReadPos_Ptr_() const{
        return BeginPtr()+readPos_;
}

const char* Buffer::WritePos_Ptr_const_() const{
        return BeginPtr()+writePos_;
}

char* Buffer::WritePos_Ptr_() {
        return BeginPtr()+writePos_;
}

void Buffer::Ensure_Writeable(size_t len) {
        if(Writeable_Bytes() < len) {
                MakeSpace(len);
        }
        assert(Writeable_Bytes() >= len);
}

void Buffer::HasWritten(size_t len) {
        writePos_+=len;
}

void Buffer::Retrieve(size_t len) {
        //直接往前走len步，当作跳过
        assert(len <= Readable_Bytes());
        readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end) {
        //把readPos后面的东西都跳过去了
        assert(ReadPos_Ptr_() <= end );
        Retrieve(end - ReadPos_Ptr_());
}

void Buffer::RetrieveAll() {
        //清空所有
        bzero(&buffer_[0], buffer_.size());
        readPos_ = 0;
        writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
        //from buffer (5)	string (const char* s, size_t n);
        //from buffer Copies the first n characters from the array of characters pointed by s.
        std::string str(ReadPos_Ptr_(), Readable_Bytes());
        RetrieveAll();
        return str;   
}

void Buffer::Append(const char* str, size_t len){
        assert(str);
        Ensure_Writeable(len);
        std::copy(str,str+len,WritePos_Ptr_());
        HasWritten(len);
}

void Buffer::Append(const std::string& str){
        Append(str.data(),str.length());
}

void Buffer::Append(const void* data,size_t len){
        assert(data);
        Append(static_cast<const char*>(data),len);
}

void Buffer::Append(const Buffer& buff){
        Append(buff.ReadPos_Ptr_(),buff.Readable_Bytes());
}


ssize_t Buffer::WriteFd(int fd,int * Errno){
        ssize_t len=write(fd,ReadPos_Ptr_(),Readable_Bytes());
        if (len < 0){
                *Errno = errno;
                return len;
        }
        readPos_+=len;
        return len;
}

ssize_t Buffer::ReadFd(int fd,int * Errno){
        char buff[65535];
        struct iovec iov[2];
        const size_t writable = Writeable_Bytes();
        /* 分散读， 保证数据全部读完 */
        iov[0].iov_base = BeginPtr() + writePos_;
        iov[0].iov_len = writable;
        iov[1].iov_base = buff;
        iov[1].iov_len = sizeof(buff);

        const ssize_t len = readv(fd, iov, 2);
        if(len < 0) {
                *Errno = errno;
        }
        else if(static_cast<size_t>(len) <= writable) {
                writePos_ += len;
        }
        else {
                writePos_ = buffer_.size();
                Append(buff, len - writable);
        }
        return len;
}

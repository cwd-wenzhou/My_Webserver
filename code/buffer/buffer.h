/*
 * @Author       : cwd
 * @Date         : 2021-4-23
 * @Place  : hust
 * 知识点：
 *      1：atomic：The main characteristic of atomic objects is that access to this contained value
 *      from different threads cannot cause data races (i.e., doing that is well-defined behavior, 
 *       with accesses properly sequenced). Generally, for all other objects, the possibility of causing 
 *      a data race for accessing the same object concurrently qualifies the operation as undefined behavior.
 * 
 *     2:  string::data   Returns a pointer to an array that contains a null-terminated sequence of 
 *              characters (i.e., a C-string) representing the current value of the string object.
 * 
 *     3:copy():
 *     The behavior of this function template is equivalent to:
 *      template<class InputIterator, class OutputIterator>
 *      OutputIterator copy (InputIterator first, InputIterator last, OutputIterator result){
 *              while (first!=last) {
 *                      *result = *first;
 *                      ++result; ++first;
 *              }
 *              return result;  
 *        }
 *      
 *      4:struct iovec
        {
                void *iov_base;	//Pointer to data.  
                size_t iov_len;	//Length of data.  
        }; 

        5:readv(fd,iov,2): 从fd描述符的文件，读到iov为头的缓冲区数组。缓冲区个数为2。缓冲区格式为iovec。
 */     
#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include<assert.h>
#include <atomic>         // std::atomic
#include <vector>
#include <cstring>
class Buffer
{


public:
        Buffer(int initBuffSize = 1024);
        ~Buffer();

        size_t Writeable_Bytes() const;
        size_t Readable_Bytes() const;
        size_t Prependable_Bytes() const;

        const char* ReadPos_Ptr_() const;
        const char* WritePos_Ptr_const_() const;
        char* WritePos_Ptr_() ;

        void Ensure_Writeable(size_t len);//确保buffer剩余空间>len，可以写入
        void HasWritten(size_t len);

        void Retrieve(size_t len);
        void RetrieveUntil(const char* end);
        void RetrieveAll();
        std::string RetrieveAllToStr();

        void Append(const char* str, size_t len);
        void Append(const std::string& str);
        void Append(const void* data, size_t len);
        void Append(const Buffer& buff);

        ssize_t ReadFd(int fd, int* Errno);
        ssize_t WriteFd(int fd, int* Errno);

private:
        std::vector<char> buffer_;//储存字符
        //使用atomic原子操作，定义当前读到的位置和写的位置
        std::atomic<std::size_t> readPos_;
        std::atomic<std::size_t> writePos_;

        const char * BeginPtr() const;
        char* BeginPtr();//因为一些函数调用要用char* 而不是const char*.写一个重载

        void MakeSpace(size_t len);//使buffer有len长度的可写空间        
};



#endif //BUFFER_H
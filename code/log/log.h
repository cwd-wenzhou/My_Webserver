/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 * 涉及知识点：
 *      1：#define 里定义函数用do {}while (0)使其成为一个语句块
 *      2：...为可变参数，#define 里的是这个## __VA_ARGS__
 */ 
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <assert.h>


#define Log_Debug(format, ...) do {Log_Base(0,format,## __VA_ARGS__)} while(0);
#define Log_Info(format, ...)      do {Log_Base(1,format,## __VA_ARGS__)} while(0);
#define Log_Warn(format, ...)   do {Log_Base(2,format,## __VA_ARGS__)} while(0);
#define Log_Error(format, ...)   do {Log_Base(3,format,## __VA_ARGS__)} while(0);

class log
{
private:
        /* data */
public:
        log(/* args */);
        ~log();
};


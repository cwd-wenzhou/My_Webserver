/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 * 涉及知识点：
 *      1：#define 里定义函数用do {}while (0)使其成为一个语句块
 *      2：...为可变参数，#define 里的是这个## __VA_ARGS__
 *      3:mkdir 在加入头文件<sys/stat.h>后可以直接调用，相当于在终端运行
 *      4：fopen(filename,"a");
 *                  "a": append: Open file for output at the end of a file. 
 *                           Output operations always write data at the end of the file, expanding it. 
 *      5:time:     
 *             time(&timer);   get current time; same as: timer = time(NULL)  
 *
 */ 
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <assert.h>
#include <mutex>
#include <thread>
#include <sys/stat.h>         //mkdir
#include <time.h>
#include "../buffer/buffer.h"
using namespace std;

#define Log_Debug(format, ...) do {Log_Base(0,format,## __VA_ARGS__)} while(0);
#define Log_Info(format, ...)      do {Log_Base(1,format,## __VA_ARGS__)} while(0);
#define Log_Warn(format, ...)   do {Log_Base(2,format,## __VA_ARGS__)} while(0);
#define Log_Error(format, ...)   do {Log_Base(3,format,## __VA_ARGS__)} while(0);

class Log
{
private:
        static const int LOG_PATH_LEN = 256;
        static const int LOG_NAME_LEN = 256;
        static const int MAX_LINES = 50000;

        int level_;//0,1,2,3指示日志紧急程度
        mutex mtx_;
        bool IsOpen_;//日志系统启动与否的标志

        const char* path_;
        const char* suffix_;

        Buffer buff_;
        FILE* fp_;
        int Day_Since_Newyear;
        //记录今天离元旦的距离，用于判断是否过了一天。过了一天了就要重开一个日志文件

        void AppendLogLevelTitle_(int level);//贴每行日志的头
public:
        Log(/* args */);
        ~Log();

        void Init(int level,const char* path="./log",const char* suffix="WritenByCWD.log");
        Log* Instance();//饿汉单例模式
        void SetLevel(int level);
        int GetLevel();

        void write();

        void flush();

        bool IsOpen(){return IsOpen_;}//返回日志系统是否已经启动


};


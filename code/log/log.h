/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 * 测试编译命令：
 *      g++ -std=c++14 -O2 -Wall -g log.cpp  ../buffer/buffer.cpp -o test -pthread -lmysqlclient
 * LOG的调用方式例子：
 *      LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
 * 
 * 涉及知识点：
 *      1：#define 里定义函数用do {}while (0)使其成为一个语句块
 *      2：...为可变参数，#define 里的是这个## __VA_ARGS__
 *      3:mkdir 在加入头文件<sys/stat.h>后可以直接调用，相当于在终端运行
 *      4：fopen(filename,"a");
 *                  "a": append: Open file for output at the end of a file. 
 *                           Output operations always write data at the end of the file, expanding it. 
 *      5:time();gettimeofday():     
 *             time(&timer);   get current time; same as: timer = time(NULL)  
 *      time 与 gettimeofday 两个函数得到的都是从Epoch开始到当前的秒数(tt=tv.tv_sec)，
 *      而gettimeofday 还能得到更精细的微秒级结果，即tv_sec*(10^6)+tv_usec为从Epoch开始到当前的微秒数
 *      6：snprintf  
 *                      int snprintf ( char * s, size_t n, const char * format, ... );
 *              vsnprintf
 *                      int vsnprintf (char * s, size_t n, const char * format, va_list arg );
 *      7:va_start
 *              void va_start (va_list ap, paramN);
 *              Initialize a variable argument list  Initializes ap to 
 *              retrieve the additional arguments after parameter paramN.
 *      8.编译warning：[-Wreorder] ：代码中的成员变量的初始化顺序和它们实际执行时初始化顺序不一致，给出警告
 */ 
#include <mutex>
#include <thread>
#include <sys/time.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "blockqueue.h"
#include "../buffer/buffer.h"
using namespace std;


#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define Log_Debug(format, ...) do {LOG_BASE(0,format,## __VA_ARGS__)} while(0);
#define Log_Info(format, ...)      do {LOG_BASE(1,format,## __VA_ARGS__)} while(0);
#define Log_Warn(format, ...)   do {LOG_BASE(2,format,## __VA_ARGS__)} while(0);
#define Log_Error(format, ...)   do {LOG_BASE(3,format,## __VA_ARGS__)} while(0);

class Log
{
private:
        Log();
        ~Log();
        
        static const int LOG_PATH_LEN = 256;
        static const int LOG_NAME_LEN = 256;
        static const int MAX_LINES = 5000;

        int level_;//0,1,2,3指示日志紧急程度
        mutex mtx_;
        bool IsOpen_;//日志系统启动与否的标志
        

        const char* path_;
        const char* suffix_;

        Buffer buff_;
        FILE* fp_;
        int Day_Since_Newyear;
        //记录今天离元旦的距离，用于判断是否过了一天。过了一天了就要重开一个日志文件

        int lineCount_;//记录所有文件已经写了的行数

        void AppendLogLevelTitle_(int level);//贴每行日志的头
        void AsyncWrite_();
        bool isAsync_;//异步输出是否开启的标志
        unique_ptr<BlockDeque<string>> deque_; 
        unique_ptr<thread> writeThread_;

public:
        static Log* Instance();//饿汉单例模式
        void Init(int level,const char* path="./log",const char* suffix="WritenByCWD.log",
                int maxQueueCapacity = 1024);
        
        void SetLevel(int level);
        int GetLevel();

        void write(int level,const char *format,...);  

        static void FlushLogThread();
        void flush();

        bool IsOpen(){return IsOpen_;}//返回日志系统是否已经启动

};


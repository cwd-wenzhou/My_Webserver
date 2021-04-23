#include "log.h"
Log::Log(/* args */)
{
}

Log::~Log()
{
}

int Log::GetLevel() {
        lock_guard<mutex> locker(mtx_);
        return level_;
}

void Log::SetLevel(int level) {
        lock_guard<mutex> locker(mtx_);
        level_ = level;
}


void Log::flush() {
//     if(isAsync_) { 
//         deque_->flush(); 
//     }
    fflush(fp_);
}

void Log::Init(int level,const char* path="./log",const char* suffix="WritenByCWD.log"){
        level_=level;
        IsOpen_=true;
        path_=path;
        suffix_=suffix;

        time_t rawtime = time(nullptr);
        struct tm *sysTime = localtime(&rawtime);
        struct tm t = *sysTime;
        //struct tm t=localtime(&rawtime);报错
        Day_Since_Newyear=t.tm_yday;//记录今天
        char fileName[LOG_NAME_LEN] = {0};
        snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
        //拼出fileName==./log/2021_04_23WritenByCWD.log 

        {
                lock_guard<mutex> locker(mtx_);
                buff_.RetrieveAll();
                if(fp_) { 
                        flush();
                        fclose(fp_); 
                }
                fp_ = fopen(fileName, "a");
                if(fp_ == nullptr) {
                        mkdir(path_, 0777);
                        fp_ = fopen(fileName, "a");
                } 
                assert(fp_ != nullptr);
        }
}

void Log::AppendLogLevelTitle_(int level) {
        switch(level) {
        case 0:
                buff_.Append("[debug]: ", 9);
                break;
        case 1:
                buff_.Append("[info] : ", 9);
                break;
        case 2:
                buff_.Append("[warn] : ", 9);
                break;
        case 3:
                buff_.Append("[error]: ", 9);
                break;
        default:
                buff_.Append("[info] : ", 9);
                break;
        }
}

Log* Log::Instance() {
    static Log inst;
    return &inst;
}
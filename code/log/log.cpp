/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 */ 
#include "log.h"
Log::Log(/* args */)
{
        lineCount_ = 0;
        isAsync_ = false;
        writeThread_ = nullptr;
        deque_ = nullptr;
        Day_Since_Newyear = 0;
        fp_ = nullptr;
}

Log::~Log()
{
        if(writeThread_ && writeThread_->joinable()) {
        while(!deque_->empty()) {
                deque_->flush();
        };
        deque_->Close();
        writeThread_->join();
        }
        if(fp_) {
                lock_guard<mutex> locker(mtx_);
                flush();
                fclose(fp_);
        }
}

Log* Log::Instance() {
    static Log inst;
    return &inst;
}

void Log::Init(int level,const char* path,const char* suffix, int maxQueueCapacity){
        level_=level;
        IsOpen_=true;
        if (maxQueueCapacity>0){
                //初始化异步情况
                isAsync_=true;
                if (!deque_){
                        //还未初始化。申请BlockDeque空间，开启写的进程
                        unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
                        deque_ = move(newDeque);//unique_ptr只能用move

                        std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));
                        writeThread_ = move(NewThread);
                }
        }
        else{
                //同步情况，即每次调用LOG系列函数，直接同时往文件里写
                //会被IO阻塞，慢一些
                isAsync_ = false;
        }

        path_=path;
        suffix_=suffix;
        lineCount_=0;

        time_t rawtime = time(nullptr);
        struct tm *sysTime = localtime(&rawtime);
        struct tm t = *sysTime;
        //struct tm t=localtime(&rawtime);报错
        Day_Since_Newyear=t.tm_yday;//记录今天
        char fileName[LOG_NAME_LEN] = {0};
        snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
        //拼出fileName==./log/2021_04_23WritenByCWD.log 

        //上个锁，打开命名的log文件，并清空内容
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
                        printf("filename==%s",fileName);
                        fp_ = fopen(fileName, "a");
                } 
                assert(fp_ != nullptr);
        }
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

void Log::write(int level,const char *format,...){
        struct timeval now = {0,0};
        gettimeofday(&now, nullptr);//用gettimeofday()获得更精确的系统时间
        //因为写日志行的时候需要微秒级的时间精度
        time_t Sec=now.tv_sec;
        struct tm *sysTime = localtime(&Sec);
        struct tm t = *sysTime;

        va_list vaList;

        /* 处理需要开新文件的情况：
                1.日志日期非本日 
                2.日志行数超过最大的行数 */
        if (Day_Since_Newyear != t.tm_yday || (lineCount_ && (lineCount_  %  MAX_LINES == 0)))
        {
                unique_lock<mutex> locker(mtx_);
                locker.unlock();
                
                char newFile[LOG_NAME_LEN];
                char tail[36] = {0};
                snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

                if (Day_Since_Newyear != t.tm_yday)
                {
                        snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
                        Day_Since_Newyear = t.tm_yday;
                        lineCount_ = 0;
                }
                else {
                        snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
                }
                
                //上锁，关老文件，开新文件
                locker.lock();
                flush();
                fclose(fp_);
                fp_ = fopen(newFile, "a");
                assert(fp_ != nullptr);
        }

        {
                //处理每次调用write往日志里写的内容
                unique_lock<mutex> locker(mtx_);
                lineCount_++;
                int n = snprintf(buff_.WritePos_Ptr_(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
                
                buff_.HasWritten(n);
                AppendLogLevelTitle_(level);

                va_start(vaList, format);
                int m = vsnprintf(buff_.WritePos_Ptr_(), buff_.Writeable_Bytes(), format, vaList);
                va_end(vaList);
                
                buff_.HasWritten(m);
                buff_.Append("\n\0", 2);

                if(isAsync_ && deque_ && !deque_->full()) {
                        //开启异步输出，队列存在且非满，把输出压到队列里
                        deque_->push_back(buff_.RetrieveAllToStr());
                } else {
                        fputs(buff_.ReadPos_Ptr_(), fp_);
                }
                buff_.RetrieveAll();
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

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}

void Log::AsyncWrite_() {
    string str = "";
    while(deque_->pop(str)) {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}


// int main(int argc, char const *argv[])
// {
        
//         Log::Instance()->Init(0);
//         //Log_Debug("num==%d",5);
//         for (int i=0;i<100000;i++){
//                 if (i%4==0)     Log_Debug("num==%d",i);
//                 if (i%4==1)     Log_Info("num==%d",i);
//                 if (i%4==2)     Log_Warn("num==%d",i);
//                 if (i%4==3)     Log_Error("num==%d",i);
//         }
//         cout<<"that's all"<<endl;
//         return 0;
// }

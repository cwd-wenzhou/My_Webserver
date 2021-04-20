/*
 * @Author       : cwd
 * @Date         : 2021-4-20
 * @Place  : hust
 */ 

/*信号量API
int sem_init(sem_t *sem, int pshared, unsigned int value);，其中sem是要初始化的信号量，pshared表示此信号量是在进程间共享还是线程间共享，value是信号量的初始值。
int sem_destroy(sem_t *sem);,其中sem是要销毁的信号量。只有用sem_init初始化的信号量才能用sem_destroy销毁。
int sem_wait(sem_t *sem);等待信号量，如果信号量的值大于0,将信号量的值减1,立即返回。如果信号量的值为0,则线程阻塞。相当于P操作。成功返回0,失败返回-1。
int sem_post(sem_t *sem); 释放信号量，让信号量的值加1。相当于V操作。
*/
#ifndef SqlConnectPool
#define SqlConnectPool

//#include "myhead.h"
#include <mysql/mysql.h>
#include <assert.h>
#include <semaphore.h>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
class Sql_Connect_Pool
{
private:
        //把构造函数，析构函数，赋值运算符和拷贝构造函数都声明为私有函数
        Sql_Connect_Pool();
        ~Sql_Connect_Pool();
        Sql_Connect_Pool(Sql_Connect_Pool const & single);
        Sql_Connect_Pool& operator = (const Sql_Connect_Pool& single);

        int Max_Connect_NUM; //最大连接数量
        int Free_Connect_NUM;//可用的连接数量
        int Used_Connect_NUM;//已用的连接数量

        std::queue<MYSQL *> Connect_Queue;//Sql连接队列
        std::mutex mtx_;
        sem_t semId_;//信号量，储存现有可用连接个数
public:
        //线程安全的懒汉式单例模式
        static Sql_Connect_Pool  *Instance(){
                static Sql_Connect_Pool Sql_Connect_Pool_Single;
                return &Sql_Connect_Pool_Single;
        };
        
        //获取Free_Connect_NUM
        int GET_Free_Connect_NUM();

        //初始化数据库链接
        void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);

        //释放数据库连接
        void Free_Connect(MYSQL* sql);

        //获取一个数据库连接
        MYSQL *Get_Connect();

        //关闭并销毁所有数据库连接
        void  ClosePool();
};

#endif //SqlConnectPool
/*
 * @Author       : cwd
 * @Date         : 2021-4-20
 * @Place  : hust
 */ 
#include  "Sql_Connect_Pool.h"
Sql_Connect_Pool::Sql_Connect_Pool(/* args */)
{
        //构造函数里面把两个连接数量置0；
        Free_Connect_NUM = 0;
        Used_Connect_NUM = 0;
}

Sql_Connect_Pool::~Sql_Connect_Pool()
{
        ClosePool();
}

//获取Free_Connect_NUM
int     Sql_Connect_Pool::GET_Free_Connect_NUM(){
        return Connect_Queue.size();
}

//初始化数据库链接
void Sql_Connect_Pool::Init(const char* host, int port,
        const char* user,const char* passwd, const char* db, int connSize){
        assert(connSize>0);
        for (int i=0;i<connSize;i++){
                MYSQL *sql = nullptr;
                sql = mysql_init(sql);
                if  (!sql){
                        printf("MySql init error. \n");
                }
                sql = mysql_real_connect(sql, host,user, passwd,db, port, nullptr, 0);
                if  (!sql){
                        printf("MySql Connect error. \n");
                }
                else printf("MySql Connect successed. \n");
                Connect_Queue.push(sql);
        }
        Max_Connect_NUM = connSize;
        sem_init(&semId_, 0, Max_Connect_NUM);//创建信号量
}

//释放数据库连接
void Sql_Connect_Pool::Free_Connect(MYSQL* sql){
        assert(sql);
        std::lock_guard<std::mutex> locker(mtx_);  //上锁
        Connect_Queue.push(sql);
        sem_post(&semId_);
}

//获取一个数据库连接
MYSQL * Sql_Connect_Pool::Get_Connect(){
        MYSQL *sql = nullptr;
        if (Connect_Queue.empty()){
                printf("SqlPool busy!\n");
                return nullptr;
        }
        sem_wait(&semId_);//信号量避免死锁
        {
                //加个括号，减少lock的代码量
                std::lock_guard<std::mutex> locker(mtx_);  //上锁
                sql=Connect_Queue.front();
                Connect_Queue.pop();
        }
        return sql;
}

//关闭并销毁所有数据库连接
void Sql_Connect_Pool::ClosePool(){
        std::lock_guard<std::mutex> locker(mtx_);
        while (!Connect_Queue.empty()){
                auto temp=Connect_Queue.front();
                Connect_Queue.pop();
                mysql_close(temp);
        }
        mysql_library_end();
}


//g++ -std=c++14 -O2 -Wall -g Sql_Connect_Pool.cpp -o test -pthread -lmysqlclient
//root用户下运行./test
int main(){
        Sql_Connect_Pool::Instance()->Init("localhost",0,"root","1","cwd_db",10);
        MYSQL* test_sql;
        printf("Sql_Connect_Pool::Instance()->GET_Free_Connect_NUM()==%d\n",
                Sql_Connect_Pool::Instance()->GET_Free_Connect_NUM());
        //std::cout<<Sql_Connect_Pool::Instance()->GET_Free_Connect_NUM()<<std::endl;
        test_sql = Sql_Connect_Pool::Instance()->Get_Connect();
        printf("mysql_info(test_sql)=%s\n",mysql_info(test_sql));
        //std::cout<<<<std::endl;
}
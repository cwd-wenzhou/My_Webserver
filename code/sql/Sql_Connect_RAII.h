/*
 * @Author       : cwd
 * @Date         : 2021-4-20
 * @Place  : hust
 */ 

#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H
#include "Sql_Connect_Pool.h"

/* 资源在对象构造初始化 资源在对象析构时释放*/
class SqlConnRAII {
public:
    SqlConnRAII(MYSQL** sql, Sql_Connect_Pool *connpool) {
        assert(connpool);
        *sql = connpool->Get_Connect();
        sql_ = *sql;
        connpool_ = connpool;
    }
    
    ~SqlConnRAII() {
        if(sql_) { connpool_->Free_Connect(sql_); }
    }
    
private:
    MYSQL *sql_;
    Sql_Connect_Pool* connpool_;
};

#endif //SQLCONNRAII_H
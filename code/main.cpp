/*
 * @Author       : cwd
 * @Date         : 2021-5-1
 * @Place  : hust
 */ 
#include <unistd.h>
#include "server/webserver.h"
#include <iostream>
using namespace std;
int main(int argc, char const *argv[])
{

#ifdef DEAMON
    daemon(1, 0); 
#endif
/*
WebServer(
                int port,int trigMode,int timeoutMS,bool OptLinger,
                int sqlPort, const char* sqlUser, const  char* sqlPwd, 
                const char* dbName, int connPoolNum, int threadNum,
                bool openLog, int logLevel, int logQueSize
        );
*/
    WebServer server(
        1316 , 3 , 60000 , false,
        3306,"root","1",
        "cwd_db",12,12,false,1,1024);
    server.Start();
    return 0;
}

/*
cwd@cwd:~/code$ webbench -c 100 -t 10 http://218.197.199.16:1316/
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Request:
GET / HTTP/1.0
User-Agent: WebBench 1.5
Host: 218.197.199.16


Runing info: 100 clients, running 10 sec.

Speed=519492 pages/min, 27991612 bytes/sec.
Requests: 86582 susceed, 0 failed.
*/


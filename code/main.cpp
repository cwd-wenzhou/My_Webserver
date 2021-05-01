/*
 * @Author       : cwd
 * @Date         : 2021-5-1
 * @Place  : hust
 */ 
#include <unistd.h>
#include "server/webserver.h"
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
        "cwd_db",12,6,true,1,1024);
    server.Start();
    return 0;
}




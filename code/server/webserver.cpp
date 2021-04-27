/*
 * @Author       : cwd
 * @Date         : 2021-4-27
 * @Place  : hust
 */ 

#include "webserver.h"

WebServer::WebServer(
        int port,int trigMode,int timeoutMS,bool OptLinger,
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize)
{
        srcDir_=getcwd(nullptr,256);//获取当前工作路径
        assert(srcDir_);
        strncat(srcDir_,"/resources/",16);//工作路径后面添上/resources/
        
        HttpConn::userCount=0;
        HttpConn::srcDir=srcDir_;

        Sql_Connect_Pool::Instance()->Init("localhost",sqlPort,sqlUser,sqlPwd,dbName,connPoolNum);

        InitEventMode_(trigMode);
        if(!InitSocket_()) { isClose_ = true;}

        //若开启log，那么开始记录
        if (openLog){
                Log::Instance()->Init(logLevel,"./log",".log",logQueSize);
                if (isClose_){Log_Error("================Server init ERROR!===========");}
                else{
                        Log_Info("========== Server init ==========");
                        Log_Info("Port:%d, OpenLinger: %s", port_, OptLinger? "true":"false");
                        Log_Info("Listen Mode: %s, OpenConn Mode: %s",
                                        (listenEvent_ & EPOLLET ? "ET": "LT"),
                                        (connEvent_ & EPOLLET ? "ET": "LT"));
                        Log_Info("LogSys level: %d", logLevel);
                        Log_Info("srcDir: %s", HttpConn::srcDir);
                        Log_Info("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
                }
        }
}

WebServer::~WebServer()
{
        close(listenFd_);
        isClose_=true;
        free(srcDir_);
        Sql_Connect_Pool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode){
        listenEvent_ = EPOLLRDHUP;
        connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
        switch (trigMode)
        {
        case 0:
                break;
        case 1:
                connEvent_ |= EPOLLET;
                break;
        case 2:
                listenEvent_ |= EPOLLET;
                break;
        case 3:
                listenEvent_ |= EPOLLET;
                connEvent_ |= EPOLLET;
                break;
        default:
                listenEvent_ |= EPOLLET;
                connEvent_ |= EPOLLET;
                break;
        }
        HttpConn::isET = (connEvent_ & EPOLLET);
}

bool WebServer::InitSocket_(){
        int ret;
        struct sockaddr_in addr;
        if (port_>65535 | port_<1024){
                Log_Error("Port:%d error",port_);
                return false;
        }

        addr.sin_family = AF_INET;//TCP/IP
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_);

        struct linger optLinger = { 0 };
        if(openLinger_) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
                optLinger.l_onoff = 1;
                optLinger.l_linger = 1;
        }

        //创建套接字描述符
        listenFd_=socket(AF_INET,SOCK_STREAM,0);
        if (listenFd_<0){
                Log_Error("create socker  %d error!",port_);
                return false;
        }

        //对该套接字设置优雅关闭
        ret=setsockopt(listenFd_, SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));
        if (ret<0){
                close(listenFd_);
                Log_Error("SET SOCKET %d SO_LINGER ERROR!!",port_);
                return false;
        }

        //对该套接字设置端口复用
        int optval=1;
        ret=setsockopt(listenFd_, SOL_SOCKET,SO_REUSEPORT,(const void*)&optval,sizeof(optval));
        if (ret<0){
                close(listenFd_);
                Log_Error("SET SOCKET %d SO_REUSEPORT ERROR!",port_);
                return false;
        }

        ret = bind(listenFd_,(struct sockaddr*)&addr,sizeof(addr));
        if(ret < 0) {
                Log_Error("Bind Port:%d error!", port_);
                close(listenFd_);
                return false;
        }

        ret = listen(listenFd_,6);
        if(ret < 0) {
                Log_Error("Listen port:%d error!", port_);
                close(listenFd_);
                return false;
        }

        ret = epoller_->AddFd(listenFd_,listenEvent_|EPOLLIN);
        if (ret ==0){
                Log_Error("Add listen ERROR！");
                close(listenFd_);
                return true;
        }
        SetFdNonblock(listenFd_);
        Log_Info("Server port:%d",port_);
        return true;
}

int WebServer::SetFdNonblock(int fd){
        assert(fd>0);
        int flags=fcntl(fd,F_GETFD,0);
        return fcntl(fd,F_SETFL,flags| O_NONBLOCK);
}

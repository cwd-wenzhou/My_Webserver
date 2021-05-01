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
        if (port_>65535 || port_<1024){
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

        //绑定套接字
        ret = bind(listenFd_,(struct sockaddr*)&addr,sizeof(addr));
        if(ret < 0) {
                Log_Error("Bind Port:%d error!", port_);
                close(listenFd_);
                return false;
        }

        //监听套接字
        ret = listen(listenFd_,6);
        if(ret < 0) {
                Log_Error("Listen port:%d error!", port_);
                close(listenFd_);
                return false;
        }

        //放到epoll里面给epoll管
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

void WebServer::AddClient_(int fd,sockaddr_in addr){
        assert(fd>0);
        users_[fd].Init(fd,addr);
        if (timeoutMS_>0){
                //若设置超时，由timer管理连接超时
                timer_->Add_Node(fd,timeoutMS_,std::bind(&WebServer::CloseConn_,this,&users_[fd]));
        }
        epoller_->AddFd(fd,EPOLLIN | connEvent_);
        SetFdNonblock(fd);
        Log_Info("Client[%d] in!", users_[fd].GetFd());
}


void WebServer::SendError_(int fd,const char*info){
        //直接向这个socket发送报错信息info 然后关掉这个fd
        assert(fd>0);
        int ret =send(fd,info,strlen(info),0);
        if (ret<0)
                //错误信息都发不过去，log里记录一下
                Log_Warn("SendError_ to client[%d] error",fd);
        close(fd);
}

//连接完成后，更新（重置）一下time里面的这个fd对应的超时的时间
void WebServer::ExtentTime_(HttpConn* client){
        assert(client);
        if(timeoutMS_ > 0) { timer_->Adjust_Node(client->GetFd(), timeoutMS_); }
}

void WebServer::CloseConn_(HttpConn* client){
        assert(client);
        Log_Info("Client[%d] quit!",client->GetFd());
        epoller_->DelFd(client->GetFd());//从epoll中删除对该连接的监管
        client->Close();
}


 void WebServer::DealListen_(){
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        do {
                int fd = accept(listenFd_,(struct sockaddr*)&addr,&len);
                if (fd<0){
                        //accept出错
                        Log_Error("accept error");
                        return;
                }
                else if (HttpConn::userCount>=MAX_FD){
                        //已连接个数达到最大值
                        SendError_(fd,"Server busy");
                        Log_Warn("Client is Full");
                        return ;
                }
                AddClient_(fd,addr);
        }while (listenEvent_ & EPOLLET);//若为ET模式，需要不停循环
 }

//更新一下超时时间，把Onwrite加入到线程池
 void WebServer::DealWrite_(HttpConn* client){
        assert(client);
        ExtentTime_(client);
        threadpool_->AddTask(std::bind(&WebServer::OnWrite_,this,client));
 }

 void WebServer::DealRead_(HttpConn* client){
        assert(client);
        ExtentTime_(client);
        threadpool_->AddTask(std::bind(&WebServer::OnRead_,this,client));
 }


void WebServer::OnRead_(HttpConn* client){
        assert(client);
        int ret = -1;
        int readErrno = 0;
        ret = client->read(&readErrno);
        if(ret <= 0 && readErrno != EAGAIN) {
                CloseConn_(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite_(HttpConn* client){
        assert(client);
        int ret = -1;
        int writeErrno = 0;
        ret = client->write(&writeErrno);
        if (client->ToWriteBytes()==0){
                //传输完成
                if (client->IsKeepAlive()){
                        OnProcess(client);
                        return;
                }
        }
        else if (ret<0){
                //对非阻塞socket而言，EAGAIN不是一种错误
                if (writeErrno == EAGAIN){
                        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
                        return;
                }
        }
        //正常情况前面都处理了，程序走到这里，说明有问题、
        //关掉client连接
        CloseConn_(client);
}


/*
EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
EPOLLOUT：表示对应的文件描述符可以写；
*/
void WebServer::OnProcess(HttpConn* client) {
        if(client->process()) {
                epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
        } else {
                epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
        }
}



/*
EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
EPOLLOUT：表示对应的文件描述符可以写；

EPOLLRDHUP ：表示对应的文件描述符读关闭；
EPOLLERR：表示对应的文件描述符发生错误；
EPOLLHUP：表示对应的文件描述符被挂断；
*/
void WebServer::Start(){
        int timeMS=-1;//epoll wait timeout == -1 无事件将阻塞
        if (!isClose_)
                Log_Info("===============SERVET  START=============");
        while (!isClose_){
                //把超时的线程都清出去
                //并获得下一次出现超时线程的时间timeMS
                if(timeoutMS_ > 0) {
                        timeMS = timer_->Get_Next_Tick();
                }

                int eventCnt = epoller_->Wait(timeMS);
                //即在下一次出现超时线程前，用epoll监管wait一下

                for (int i=0;i<eventCnt;i++){
                        //处理epoll里wait到的事件
                        int fd = epoller_->GetEventFd(i);
                        uint32_t events=epoller_->GetEvents(i);

                        if (fd == listenFd_){
                                DealListen_();
                        }
                        else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                                assert(users_.count(fd)>0);
                                CloseConn_(&users_[fd]);
                                //关掉了应该删掉&users_[fd]
                                users_.erase(fd);
                        }
                        else if (events & EPOLLIN){
                                assert(users_.count(fd)>0);
                                DealRead_(&users_[fd]);
                        }
                        else if (events & EPOLLOUT){
                                assert(users_.count(fd)>0);
                                DealWrite_(&users_[fd]);
                        }
                        else 
                                Log_Error("Unexpected event");
                }

        }
}

int WebServer::SetFdNonblock(int fd){
        assert(fd>0);
        int flags=fcntl(fd,F_GETFD,0);
        return fcntl(fd,F_SETFL,flags| O_NONBLOCK);
}
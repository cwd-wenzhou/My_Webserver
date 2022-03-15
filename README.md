# My_Webserver

用C++实现的高性能WEB服务器，经过webbenchh压力测试可以实现上万的QPS

部分源码借鉴自https://github.com/markparticle/WebServer 

此仓库仅供个人学习使用

## 功能

* 利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型；
* 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求；
* 利用标准库容器封装char，实现自动增长的缓冲区；
* 基于小根堆实现的定时器，关闭超时的非活动连接；
* 利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态；
* 利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能。
* 增加log,mysql的测试程序 

## 环境要求

* Linux
* C++14
* MySql

## 目录树

```
.
├── code           源代码
│   ├── buffer
│   ├── http
│   ├── log
│   ├── timer
│   ├── pool
│   ├── sql
│   ├── Threadpool
│   ├── server
│   └── main.cpp
├── resources      静态资源
│   ├── index.html
│   ├── image
│   ├── video
│   ├── js
│   └── css
├── bin            可执行文件
│   └── server
├── log            日志文件
├── webbench-1.5   压力测试
├── build          
│   └── Makefile
├── Makefile
├── LICENSE
└── readme.md
```


## 项目启动
```bash
#安装mysql
sudo apt install mysql-server
#在18下可能出现登陆问题，要修改文件然后改密码然后登陆

#安装c++ mysql api库
sudo apt install libmysqlclient-dev
```
需要先配置好对应的数据库

```bash
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

```bash
cd build/
make
cd ..
./server
#亦可安装
make install
新终端中server即可运行，其实install就是把可执行文件放到bin目录下
```

## 单元测试

```bash
cd test
make
./test
```

## 压力测试

```bash
#测试环境
cwd@cwd:~$ lscpu
架构：           x86_64
CPU 运行模式：   32-bit, 64-bit
字节序：         Little Endian
CPU:             12
在线 CPU 列表：  0-11
每个核的线程数： 2
每个座的核数：   6
座：             1
NUMA 节点：      1
厂商 ID：        GenuineIntel
CPU 系列：       6
型号：           158
型号名称：       Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz
步进：           10
CPU MHz：        3995.444
CPU 最大 MHz：   4600.0000
CPU 最小 MHz：   800.0000
BogoMIPS：       6399.96
虚拟化：         VT-x
L1d 缓存：       32K
L1i 缓存：       32K
L2 缓存：        256K
L3 缓存：        12288K
NUMA 节点0 CPU： 0-11

webbench -c 1000 -t 100 http://www.baidu.com/
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://www.baidu.com/
1000 clients, running 100 sec.

Speed=989 pages/min, 4622061 bytes/sec.
Requests: 1255 susceed, 394 failed.

webbench -c 1000 -t 100 http://127.0.0.1:1316/
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://127.0.0.1:1316/
1000 clients, running 100 sec.

Speed=846920 pages/min, 2685188 bytes/sec.
Requests: 1411533 susceed, 1 failed.

#将其部署到阿里云上
#阿里云配置4核 8 GiB：
webbench -c 1000 -t 10 http://116.62.35.197:1316/ 
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://116.62.35.197:1316/
1000 clients, running 10 sec.

Speed=16122 pages/min, 58373 bytes/sec.
Requests: 2687 susceed, 0 failed.

```
## 致谢

[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)

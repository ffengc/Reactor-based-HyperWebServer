# 基于Reactor的高性能网络服务器

一个基于多路复用Reactor模式的高性能web服务器，底层实现使用epoll模型。

我将继续更新这个项目，继续改进后端和前端的问题，并创建一个完整的项目。到目前为止，该项目已经完成了最基本的后端建设和简单的前端建设。

**其中，epoll在本项目中的基本原理和多路复用高性能IO的基本原理可以在我的其他repo中看到**
**https://github.com/Yufccode/Multiplexing-high-performance-IO-server**

## 1. 实现效果


**后端效果**

![](./figs/1.png)

在后端可以在`stdout`中看到获取到的http请求报文，当然，我们也可以在日志文件中看到http报文。

**前端效果**

为个人博客展示。

![](./figs/2.png)

## 2. 运行方法

**环境**

- Linux ALiCentos7 3.10.0-1160.88.1.el7.x86_64 #1 SMP Tue Mar 7 15:41:52 UTC 2023 x86_64 x86_64 x86_64 GNU/Linux

- gcc (GCC) 8.3.1 20190311 (Red Hat 8.3.1-3)

克隆该项目。

```bash
git clone https://github.com/Yufccode/Reactor-based-HyperWebServer.git
```

进入该项目的文件夹中。

```bash
cd Reactor-based-HyperWebServer
```

生成可执行。

```bash
make clean;
make;
```

运行服务器。

```
./WebServer
```

注意：如果是云服务器运行本项目需要开放`8080`端口号，否则会因为防火墙而无法访问。

打开浏览器，访问服务端。

一、本机访问

输入url。

```url
127.0.0.1:8080
```

二、云服务器访问。

输入url。

```url
(云服务器ip):8080 # xxx.xxx.xxx.xxx:8080
```

## 3. 文件结构

```bash
yufc@ALiCentos7:~/Src/Bit-Project/WebServer-reactor$ tree .
.
├── CleanLogs.sh # 清除所有日志文件内容脚本
├── Logs
│   ├── Requests.log # 收到HTTP报文保存的日志文件
│   └── WebServer.log # 服务器打印的日志保存文件
├── makefile
├── Reactor # Reactor模式底层服务器文件
│   ├── Epoll.hpp
│   ├── Log.hpp
│   ├── Protocol.hpp
│   ├── Reactor-server.hpp
│   └── Sock.hpp
├── README.md
├── start-main.cc
├── text
│   └── ziliao.txt # 资料
├── tools
│   ├── main.cc-backup # main函数备份
│   └── ulity.hpp # 工具相关接口头文件
├── WebServer # 可执行程序
├── WebServer.hpp # Web服务器头文件
├── wwwroot # 前端根目录
│   ├── error
│   │   └── 404.html
│   └── index.html
└── wwwroot-backup # 前端根目录的一些备份
    ├── my_blog_root
    │   └── index.html
    └── wwwroot
        ├── error
        │   └── 404.html
        └── index.html

10 directories, 21 files
yufc@ALiCentos7:~/Src/Bit-Project/WebServer-reactor$
```

## 4. 项目原理简介

本项目基于Socker编程，采用epoll形式的多路转接，搭建了一个Reactor模式的网络服务器。

其中`WebServer.hpp`为底层`Reactor`服务器的封装。

本项目其实是Nginx服务器的核心所在。

关于Nginx等实现原理，异步IO的原理，多路转接的原理，可以见以下链接。

- **[中文-introduction](./introduction-cn.md)**

- **[English-introduction](./introduction.md)**

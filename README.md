# High-performance web server based on Reactor

- **[简体中文](./README-CN.md)**
- **[English](./README.md)**

***

- [High-performance web server based on Reactor](#high-performance-web-server-based-on-reactor)
  - [Overall structure of Reactor server](#overall-structure-of-reactor-server)
  - [Project details](#project-details)
    - [epoll\_server](#epoll_server)
    - [reactor\_server](#reactor_server)
  - [Introduction to project principles](#introduction-to-project-principles)


A high-performance web server based on the multiplexed Reactor pattern, the underlying implementation uses the epoll model.

I will continue to update this project, continue to improve the backend and frontend issues, and create a complete project. So far, the project has completed the most basic back-end construction and simple front-end construction.

This is the v2.0 version. Here, in April 2024, I remade this project and the code was optimized to a certain extent.

**Older versions can be found in Release: v1.0**

**Among them, the basic principles of epoll in this project and the basic principles of multiplexing high-performance IO can be seen in my other repo**
**https://github.com/Yufccode/Multiplexing-high-performance-IO-server**

## Overall structure of Reactor server

![](./assets/server-struct.png)

## Project details

Two servers are implemented in the project.

### epoll_server

By encapsulating the epoll feature of Linux, an epoll-based network server is implemented. This network server does not have any encapsulation and can be tested through telnet.

**Implementation details and code explanations can be found at: [work_epoll.md](./docs/work_epoll.md)**

### reactor_server

On the basis of multi-channel transfer, the server is further encapsulated and the dispatch feature is added to realize the reactor mode web server.

- The backend adopts epoll's multi-pass listening mode
- Encapsulation using reactor mode
- Added web server function, which can analyze http requests and build http responses
- Add a simple front-end page as a test

**Implementation details and code explanations can be found at: [work_reactor.md](./docs/work_reactor.md)**


## Introduction to project principles

This project is based on Socker programming and uses epoll-style multi-way transfer to build a Reactor mode network server.

This project is actually the core of the Nginx server.

For information on the implementation principles of Nginx, the principles of asynchronous IO, and the principles of multiplexing, please see the following link.

- **[中文-introduction](./docs/introduction-cn.md)**

- **[English-introduction](./docs/introduction.md)**
# Reactor-based-HyperWebServer
A high-performance web server based on the multiplexer actor mode, with an underlying implementation using the epoll model

I will continue to update this project, continue to improve the backend and front-end issues, and create a complete project. So far, this project has completed the most basic backend construction and simple front-end construction.

**Among them, the basic principles of epoll in this project and the basic principles of multiplexing high-performance IO can be seen in my other repo.**
https://github.com/Yufccode/Multiplexing-high-performance-IO-server

# What are Apache and Nginx? |What are Nginx and Reactors? |The Essence of Network IO | Blocking Queue | Asynchronous Non Blocking IO

## What is IO and what is the essence of IO?

To clarify this issue, we first need to understand the 5 most important IO models!

-Blocking IO

-Non blocking IO

-Signal driven IO

-Multiplex switching

-Asynchronous IO

### Five IO Models and Some Basic Concepts

#### Blocking IO

The system call will wait until the kernel prepares the dataAll sockets default to blocking mode. 

To put it simply, I need to read from a File descriptor. Since there is no data in the File descriptor, I have been blocking and waiting!

#### Non blocking IO

If the kernel has not prepared the data yet, the system call will still return directly and return the EWOULDBLOCK error code. 

To put it simply, I read from a File descriptor. If there is data, I will successfully read it and return it. If there is no data, I will also return it with the EWOULDBLOCK error code. 

In this way, when reading, we must check every once in a while to see whether the File descriptor has data. In fact, this is what we often call a non blocking polling detection scheme!

Polling generally consumes CPU resources, so pure non blocking IO is generally only used in specific scenarios. 

#### Signal driven IO

When the kernel prepares the data, it uses the SIGIO signal to notify the application to perform IO operations. 

#### Multiplex switching

Multiplexing I/O is a technology used to manage multiple I/O operations. It allows a single thread or process to simultaneously monitor and process multiple I/O events without creating a separate thread or process for each I/O operation. 

Multiplexing I/O utilizes mechanisms provided by the operating system, such as select, poll, epoll (Linux), or kqueue (FreeBSD, Mac OS X), to simultaneously monitor the status of multiple I/O events. 

These mechanisms allow programs to register multiple I/O events, such as socket read and write events, into a collection of events. Then, the program can call specific system calls, such as select or epoll_Wait to block and wait for any of the I/O events to be ready. 

Once a ready I/O event occurs, the program can know which I/O operations are ready through the event set and process them. This approach avoids the situation of blocking and waiting for a single I/O operation to complete, improving concurrent processing capability and efficiency. 

Multi channel switching I/O is suitable for situations where multiple I/O events need to be processed simultaneously, especially in scenarios such as network servers, message queues, and real-time stream processing. By using multipath I/O, the creation and switching overhead of threads or processes can be reduced, and the performance and resource utilization of the system can be improved. 

**In general, it is a process. I can listen to multiple File descriptor at the same time. When a specific event of a File descriptor is ready, it will remind the upper layer**

#### Asynchronous IO

The key concepts of Asynchronous I/O are callback and event loop. In the Asynchronous I/O model, when a program initiates an I/O operation, it will register a callback function and return control to the caller. When the I/O operation is completed, the system will notify the program and call the pre registered callback function at the appropriate time. The program can process the results of completed I/O operations in the callback function. 

The advantage of Asynchronous I/O is that it can continue to perform other tasks while waiting for I/O operations to complete, which improves the concurrent processing capability and system response performance. Since there is no need to create additional threads or processes for each I/O operation, the Asynchronous I/O model consumes less system resources. 

**For example, Reactor is an asynchronous IO application, and the content about Reactor will be explained by the blogger in the following content**

#### Synchronous communication vs Asynchronous communication

Synchronous and asynchronous focus on message communication mechanisms

The so-called synchronization is that when a call is made, it does not return until a result is obtained. But once the call returns, the return value is obtained; In other words, the caller actively waits for the result of this call. 

Asynchronous is the opposite. After the call is made, it returns directly, so there is no result returned. In other words, when an asynchronous procedure call is issued, the caller will not get the result immediately. Instead, after the call is issued, the callee notifies the caller through status and notification, or processes the call through callback functions. 

**This is not the same concept as synchronous mutual exclusion with multiple threads, so don't confuse it**

#### Blocking queue

**Blocking queues are actually a method of synchronously blocking IO applications**

🔗

**Of course, similar to this, there is also a circular queue, but of course, the essence is the same because they all block**

### What is inefficient IO?What is efficient IO?

From the above description, we can conclude a very, very important conclusion: 

**IO=wait+copy data (copy data from File descriptor or to File descriptor)**

This conclusion is very important!

So, based on this conclusion, how can we improve the efficiency of IO?Or, what is efficient IO?What is inefficient?

**Answer: Reduce the proportion of waiting**

How to reduce the proportion of wait?The most effective method is actually multiplexing. 

## Multiplex switching

Among them, what is worth learning is the select, poll, and epoll of multi channel switching. Among them, epoll is the most mature. 

**I have already created a Github project for these three multi-channel transfer methods. You can transfer them to download the code and view the relevant principles and differences**

🔗

By utilizing multiplexing, we can greatly improve the efficiency of IO. 

## Why are these IO models needed?

But how can we use the IO models mentioned above?Why do we pursue ultimate efficiency?Why do we need our IO to be fast?

Here, some students will ask, I usually do pipeline testing on my own computer, and do information transmission between File descriptor, which is very fast. The transmission is completed in an instant, and there is no "waiting". Why do I do these complex IO models?

In fact, locally, we may not see the advantages of IO models, but in network scenarios, efficient IO models are very important!

The network can cause packet loss!There will be a delay!It will go wrong!Why else would there be TCP and other protocols?

Based on the above IO models, two main network service models will actually be derived, Apache and Nginx. The specific content can be found in the blogger's next blog post!

## Apache and Nginx

### The underlying principles of Apache HTTP server

The Apache HTTP server adopts the classic multi process/multi threaded model. Its main components include Master Process/thread and Worker Process/thread. When Apache starts, the main process is first created and listens to the specified port, waiting for the client to connect. When a new connection request arrives, the main process accepts the connection and assigns it to an available worker process. 

Simply put, it means multiple threads or processes, usually using threads because they occupy less CPU resources. It is actually a main thread that manages some new threads in a unified manner. The server starts and a main thread is created. If the underlying link is created, a new thread is created and accept is called. This new thread is specifically used to handle this specific link until it is closed. By creating a link, the main thread creates a new thread, essentially using the new thread to take on tasks. Of course, this scheme can be optimized. Generally, a Thread pool will be built to complete these things. First, a bunch of threads will be created, and then a thread will be linked. Then, a thread will pick up the task. If there are not enough threads in the Thread pool, you can choose to create, block, and release other threads. 

So where is the problem with this method?If a link comes, it's a long link, and I don't leave or send you messages, what about your tasking thread?It can only be suspended by the link occupying the pit and not working, neither releasing nor working. If there are many long links like this, it is very CPU intensive!

### The underlying principles of Nginx

Nginx adopts an event driven, asynchronous single process model. Its underlying structure consists of multiple modules, including event module, HTTP module, Reverse proxy module, etc. 

The core component is the event module, which uses the Asynchronous I/O mechanism provided by the operating system (such as epoll, kqueue) to achieve efficient event processing. The main process of Nginx is an event driven actor that listens and accepts client connections through an Event Loop. When a new connection arrives, the main process will distribute the connection to an available worker process. 

The Worker Process is the executor of Nginx's actual processing of requests. Each worker process is independent and shares the same event loop across multiple connections. The workflow processes requests in an event driven manner, including reading requests, parsing request headers, processing request logic, and generating responses. In the process of processing requests, Nginx uses non blocking I/O operations and makes full use of Asynchronous I/O mechanism to improve concurrent processing capability. 

#### Reactor (using epoll under Linux as an example)

Reactor is the core component of Nginx, and I will give an example to explain what event loops are and what listening is. Just give an example, and everyone will understand. For an HTTP request: 

For a server, there must be a listening socket listensocket. After the server is turned on, there will definitely be many links from other places who want to shake hands with our server three times. Therefore, we have connections and should go to accept, right?But now in epoll multiplexing mode, it is not possible to directly have the listenlock accept!Why, because I don't know when to connect? If the link doesn't come, I call accept and it blocks**Therefore, we should put the listensock in epoll to register!!Then return directly without blocking**After registration, if the link comes, that is to say, the read event of the listenlock socket (the essence of a socket is a File descriptor, and the blogger will not repeat these basic concepts) is ready!Epoll will notify me!I will go to accept again at this time, and it will definitely not be blocked!Because epoll told me that the listensock read event is ready!

So we know that sockets after accept, also known as regular sockets, may send us messages. So, can we simply call read according to the previous method?It's definitely not possible!If there's no news, why are you reading? If there's no news, isn't your reading blocked?There cannot be such low-level operations in epoll. Therefore, similarly, register in epoll!When did the news come? EPOLL told you, so you don't have to worry and just return. 

The entire process divides a request into multiple stages, each of which is registered and processed in many modules, and all operations are asynchronous and non blocking. Asynchronous here means that the server does not need to wait for the return result after executing a task, but automatically receives notifications after completion. 

**The entire process is single process and single thread, but high concurrency!I'm not afraid of long links coming. You just register in epoll, and if you don't receive messages, I won't spend time on you (calling read), so this method is very efficient**This approach enables the server to efficiently handle multiple concurrent requests and perform other tasks while waiting for I/O operations, thereby improving overall performance. 

#### What is the underlying layer of epoll?What are the advantages compared to select and poll?

🔗

#### Does reactor only have epoll?

The Reactor pattern is a design pattern used to build event driven applications. In Reactor mode, there is an Event Loop responsible for listening to events and scheduling corresponding handlers. The specific underlying implementation can adopt various technologies and system calls, among which epoll is one of the commonly used event notification mechanisms in Linux systems. 

In Linux systems, epoll provides an efficient I/O event notification mechanism, allowing servers to handle a large number of concurrent connections. Therefore, many implementations of the Reactor pattern choose to use epoll as the underlying event notification mechanism to achieve high-performance event driven. 

However, the underlying implementation of the Reactor pattern is not limited to epoll. It can also use other event notification mechanisms such as select, poll, etc., or use corresponding mechanisms on other operating systems such as kqueue (on FreeBSD and Mac OS X) or IOCP (on Windows). 

Therefore, the Reactor pattern does not rely on specific underlying implementations, but rather focuses on event driven design ideas and patterns. The specific underlying implementation depends on the operating system and the event notification mechanism chosen by the developer. 

#### Some other features of Nginx

In addition, Nginx also provides a powerful modular architecture, allowing users to choose and configure different modules according to their needs. Nginx modules can implement functions such as load balancing, caching, Reverse proxy, SSL/TLS encryption, etc. Modules can be loaded and configured through configuration files, making Nginx highly flexible and scalable. 

## An HTTP Server Based on the Reactor Model

Recently, the blogger has been working on an HTTP server based on Reactor asynchronous IO and implemented through low-level multiplexing, which can meet the requirements of high efficiency. 

The backend of this project has been basically improved, and some details are still being improved. I hope everyone can support this project more

## Summary

Both Nginx and Squid Reverse proxy servers adopt the event driven network mode. Event driven is actually an ancient technology that used early mechanisms such as select and poll. Subsequently, more advanced event mechanisms based on kernel notifications emerged, such as epoll in libevent, which improved event driven performance. The core of event driven is still I/O events. Applications can quickly switch between multiple I/O handles to achieve the so-called Asynchronous I/O. The event driven server is very suitable for handling I/O intensive tasks, such as the Reverse proxy, which acts as a data transfer station between the client and the Web server, and only involves pure I/O operations, not complex computing. It is a better choice to use event driven to build a Reverse proxy. A worker process can run without the overhead of managing processes and threads. At the same time, CPU and memory consumption are also small. 

Therefore, servers such as Nginx and Squid are implemented in this way. Of course, Nginx can also adopt a multi process and event driven mode, where several processes run libevent without requiring hundreds of processes like Apache. Nginx also performs well in handling static files, because static files themselves belong to disk I/O operations and are handled in the same way. As for the so-called tens of thousands of concurrent connections, this does not make much sense. Writing a network program casually can handle tens of thousands of concurrent connections, but if most clients are blocked somewhere, there is not much value. 

Let's take a look at application servers like Apache or Resin. They are called application servers because they need to run specific business applications, such as scientific computing, graphics and image processing, database reading and writing, etc. They are likely CPU intensive services, and event driven is not suitable for such situations. For example, if a calculation takes 2 seconds, then these 2 seconds will completely block the process and the event mechanism will have no effect. Imagine if MySQL were to switch to event driven, a large join or sort operation would block all clients. In this case, multiple processes or threads exhibit advantages, as each process can independently execute tasks without blocking or interfering with each other. Of course, modern CPUs are getting faster and faster, and the blocking time of individual calculations may be very short, but as long as there is blocking, event programming does not have an advantage. Therefore, technologies such as processes and threads will not disappear, but will complement event mechanisms and exist for a long time. 

In summary, event driven is suitable for I/O intensive services, while multiprocessing or multithreading is suitable for CPU intensive services. They each have their own advantages and there is no trend to replace each other. 

**Reference for this paragraph:**

**Copyright Notice: This article is the original work of CSDN blogger "Xi Feijian" and follows the CC 4.0 BY-SA copyright agreement. Please attach the original source link and this statement when reprinting**

**Original link: https://blog.csdn.net/xifeijian/article/details/17385831**

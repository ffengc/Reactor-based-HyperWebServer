
# Apache和Nginx是什么？｜Nginx和Reactor是什么？｜网路IO的本质｜阻塞队列｜异步非阻塞IO

**[English](./introduction.md)**

## 什么是IO，IO的本质是什么？ 

要搞清楚这个问题，我们首先要了解5种最重要的IO模型！

- 阻塞IO
- 非阻塞IO
- 信号驱动IO
- 多路转接
- 异步IO

### 五种IO模型和一些基本概念

#### 阻塞IO

在内核将数据准备好之前,系统调用会一直等待.所有的套接字,默认都是阻塞方式。

简单来说，我要向一个文件描述符做读取这个操作，这个文件描述符里面没有数据，我就一直阻塞等待！

#### 非阻塞IO 

如果内核还未将数据准备好,系统调用仍然会直接返回,并且返回EWOULDBLOCK错误码。

简单来说，我向一个文件描述符做读取，如果有数据，则成功读取返回，如果没有数据，也返回，但带上EWOULDBLOCK错误码。

使用这种方式的话，我们做读取，就必须每隔一段时间去看看，这个文件描述符到底来数据没有，这个其实就是我们常说的非阻塞轮询检测方案！

轮询一般比较吃CPU资源，因此纯非阻塞IO一般只在特定的场景下使用。

#### 信号驱动IO

内核将数据准备好的时候，使用SIGIO信号通知应用程序进行IO操作。

#### 多路转接

多路转接I/O（Multiplexing I/O）是一种用于管理多个I/O操作的技术。它允许单个线程或进程同时监视和处理多个I/O事件，而无需为每个I/O操作创建单独的线程或进程。

而多路转接I/O利用了操作系统提供的一些机制，如select、poll、epoll（Linux）或kqueue（FreeBSD、Mac OS X），来同时监视多个I/O事件的状态。

这些机制允许程序将多个I/O事件（如套接字的读写事件）注册到一个事件集合中。然后，程序可以通过调用特定的系统调用，如select或epoll_wait，来阻塞等待其中任何一个I/O事件就绪。

一旦有就绪的I/O事件发生，程序就可以通过事件集合得知是哪些I/O操作已经就绪，并对它们进行处理。这种方式避免了阻塞等待单个I/O操作完成的情况，提高了并发处理能力和效率。

多路转接I/O适用于需要同时处理多个I/O事件的情况，特别适用于网络服务器、消息队列、实时流处理等场景。通过使用多路转接I/O，可以减少线程或进程的创建和切换开销，提高系统的性能和资源利用率。

**其实通俗来说，就是一个进程，我可以同时监听多个文件描述符，哪一个文件描述符的特定事件就绪了，就提醒上层。**

#### 异步IO

异步I/O的关键概念是回调（Callback）和事件循环（Event Loop）。在异步I/O模型中，当程序发起一个I/O操作时，它会注册一个回调函数，并将控制权返回给调用者。当I/O操作完成时，系统会通知程序，并在适当的时机调用事先注册的回调函数。程序可以在回调函数中处理已完成的I/O操作的结果。

异步I/O的优势在于可以在等待I/O操作完成的同时继续执行其他任务，提高了并发处理能力和系统的响应性能。由于无需为每个I/O操作创建额外的线程或进程，异步I/O模型对系统资源的消耗也较低。

**比如说Reactor就是一种异步IO的应用，关于Reactor的内容，博主会在后面的内容中进行讲解**

#### 同步通信 vs 异步通信

同步和异步关注的是消息通信机制

所谓同步，就是在发出一个调用时，在没有得到结果之前，该调用就不返回。但是一旦调用返回，就得到返回值了;换句话说，就是由调用者主动等待这个调用的结果。

异步则是相反，调用在发出之后，这个调用就直接返回了，所以没有返回结果。换句话说，当一个异步过程调用发出后，调用者不会立刻得到结果，而是在调用发出后，被调用者通过状态、通知来通知调用者，或通过回调函数处理这个调用。

**这个和多线程的同步互斥，不是同一个概念，不要大家不要搞混了。**

#### 阻塞队列

**阻塞队列其实就是一种同步阻塞IO的应用方式。**

🔗 [多线程｜基于阻塞队列和环形队列的生产者消费者模型架构](https://blog.csdn.net/Yu_Cblog/article/details/130422047?spm=1001.2014.3001.5501)

**当然，和这个类似还有环形队列，当然其实本质都一样，因为他们都会阻塞！！**

### 什么是低效的IO？什么是高效的IO？

我们通过上面的描述，已经可以总结出一个非常非常非常重要的结论：

**IO = 等 + 拷贝数据（把数据从文件描述符里拷贝出来或拷贝到文件描述符里去）**

这个结论非常的重要！

那么通过这个结论，我们如何提高IO的效率呢？或者说，什么是高效的IO？什么低效呢？

**答案：让等的比重降低！！！！！**

如何让等的比重降低？最有效的方法其实就是多路转接了。

## 多路转接

其中，值得我们学习的，就是多路转接的select，poll和epoll。其中epoll是最成熟的。

**关于这三种多路转接的方式，博主已经做成Github的项目了，大家可以转接上去下载代码和查看相关的原理和区别。**

🔗 [Multiplexing-high-performance-IO-server](https://github.com/Yufccode/Multiplexing-high-performance-IO-server)

利用多路转接，我们就可以大大提高IO的效率了。

## 为什么需要这些IO模型？

但是上面提到的IO模型，怎么把他们用起来呢？我们为什么要追求极致的效率？为什么我们需要我们的IO很快？

这里会有同学问，我平时在自己电脑上做管道的测试，做文件描述符之间的信息传输，都很快啊，都瞬间传输完成啊，也没有怎么“等”啊，为什么要搞上面这些复杂的IO模型？

其实，本地上，我们肯定看不到IO模型的优势，但是在网络场景下，高效的IO模型就非常重要了！

网络是会丢包的！是会延时的！是会出错的！不然为什么会有TCP这些协议呢？

针对于上面这些IO模型，其实就会衍生出两种主要的网络服务模型，Apache和Nginx。具体内容可以看博主的下一篇博客！

## Apache和Nginx

### Apache HTTP服务器的底层原理

Apache HTTP服务器采用了经典的多进程/多线程模型。它的主要组件包括主进程/线程（Master Process/thread）和工作进程线程（Worker Process/thread）。当Apache启动时，主进程首先被创建，并监听指定的端口，等待客户端连接。当有新的连接请求到达时，主进程接受该连接并将其分派给一个可用的工作进程。

通俗来说，就是多线程或者多进程，当然一般是用线程的，因为线程占用cpu资源少。其实就是一个主线程，在统一管理一些新线程。服务器启动，一个主线程被创建，如果底层来链接了，则创建一个新线程，调用accept，则这个新线程专门用于处理这一个特定的链接，直到链接关闭。来一个链接，主线程就创建一个新线程，本质就是用新线程去接任务的道理。当然这种方案是可以优化的，一般会做一个线程池去完成这些事情，先创建好一堆线程，然后来链接了，则一个线程去领任务，如果线程池线程不够了，可以选择创建，或者阻塞等其他线程释放。

那么这种方法问题在哪呢？如果一个链接来了，是一个长链接，我赖着不走，也不给你发消息，那你这领任务线程咋办？就只能被这个占着坑位不干活的链接吊着，既不能释放，也不能工作。如果这样的长链接很多，这是非常吃CPU资源的！

### Nginx的底层原理

 Nginx采用了事件驱动的、异步的单进程模型。它的底层结构由多个模块组成，包括事件模块、HTTP模块、反向代理模块等。

核心组件是事件模块，它利用操作系统提供的异步I/O机制（如epoll、kqueue）来实现高效的事件处理。Nginx的主进程是一个事件驱动的Reactor，通过事件循环（Event Loop）监听和接受客户端连接。当有新的连接到达时，主进程会将连接分发给一个可用的工作进程。

工作进程（Worker Process）是Nginx实际处理请求的执行者。每个工作进程都是独立的，并且在多个连接间共享相同的事件循环。工作进程通过事件驱动的方式处理请求，包括读取请求、解析请求头、处理请求逻辑、生成响应等。在处理请求的过程中，Nginx使用非阻塞I/O操作，充分利用异步I/O机制来提高并发处理能力。

#### reactor（用linux下的epoll为例）

其中reactor就是Nginx的核心构件，我直接举一个例子解释，什么是事件循环，什么是监听这些东西。直接举例子，大家就明白了。对于一个HTTP请求而言：

对于一个服务器，肯定有一个监听套接字listensock。当服务器开启之后，肯定会有其他地方来的许多链接，想和我们这个服务器三次握手，因此我们有连接来了，应该去accept对吧？但是现在在epoll多路转接模式下，不能直接让listensock去accept！为什么，因为我不知道什么时候来链接啊，如果链接没来，我调用accept不就阻塞了？**因此，我们应该把listensock放到epoll里面去注册！！然后直接返回，不用阻塞！**注册好之后，如果链接来了，也就是说listensock套接字（套接字的本质就是文件描述符，这些基本的概念博主也不赘述了）的读事件就绪了！epoll就会通知我！我此时再去accept，此时是一定不会被阻塞了！因为epoll告诉我，listensock的读事件已经就绪了！

那么我们知道accept之后的套接字，也就是普通套接字，可能会给我们发消息的，那么按照之前的方法，直接调用read行吗？肯定是不行的！没消息来你read什么，没消息你read不就阻塞了？epoll里面不能有这么低级的操作。因此，同样，注册到epoll里面去！啥时候来消息了，epoll告诉你，你就不用管了，直接返回。

整个过程将一个请求分成了多个阶段，**每个阶段都会在许多模块中注册并进行处理**，而且所有的操作都是异步非阻塞的。异步在这里表示服务器执行一个任务后无需等待返回结果，而是在完成后自动接收通知。

**整个过程是单进程单线程的，但是高并发！长链接来了我不怕啊，你只是注册在epoll里面，你不来消息，我就不在你身上花时间（调用read），因此这种方式非常的高效！！！！！**这种方式使得服务器能够高效处理多个并发请求，并能在等待I/O操作期间执行其他任务，以提高整体性能。

#### epoll的底层是什么？相比于select和poll的优势在哪？

🔗 [Multiplexing-high-performance-IO-server](https://github.com/Yufccode/Multiplexing-high-performance-IO-server)

#### reactor只有epoll吗？

Reactor模式是一种设计模式，用于构建事件驱动的应用程序。在Reactor模式中，有一个事件循环（Event Loop）负责监听事件并调度对应的处理程序。具体的底层实现可以采用多种技术和系统调用，其中epoll是Linux系统下常用的事件通知机制之一。

在Linux系统中，epoll提供了高效的I/O事件通知机制，使得服务器能够处理大量的并发连接。因此，很多Reactor模式的实现会选择使用epoll作为底层的事件通知机制，以实现高性能的事件驱动。

然而，Reactor模式的底层实现并不仅限于epoll，它也可以使用其他的事件通知机制，如select、poll等，或者在其他操作系统上使用相应的机制，如kqueue（在FreeBSD和Mac OS X上）或IOCP（在Windows上）。

因此，Reactor模式并不依赖于特定的底层实现，而是关注于事件驱动的设计思想和模式。具体的底层实现取决于操作系统和开发者选择的事件通知机制。

#### Nginx的一些其他功能

此外，Nginx还提供了强大的模块化架构，用户可以根据需求选择和配置不同的模块。Nginx的模块可以实现诸如负载均衡、缓存、反向代理、SSL/TLS加密等功能。模块可以通过配置文件进行加载和配置，使得Nginx具有很高的灵活性和可扩展性。

## 一个基于Reactor模型的Http服务器(本项目)

最近博主就在做一个这样的http服务器，基于Reactor异步IO，底层多路转接的方式实现的，可以达到高效率的要求。

现在这个项目的后端已经基本完善了，现在还在完善一些细节，希望大家多多支持这个项目～～

## 总结

无论是Nginx还是Squid等反向代理服务器，它们都采用了事件驱动的网络模式。事件驱动实际上是一项古老的技术，早期使用的是select和poll等机制。随后，基于内核通知的更高级事件机制出现，例如libevent中的epoll，这提高了事件驱动的性能。事件驱动的核心仍然是I/O事件，应用程序能够快速切换在多个I/O句柄之间，实现所谓的异步I/O。事件驱动服务器非常适合处理I/O密集型任务，例如反向代理，它充当客户端和Web服务器之间的数据中转站，仅涉及纯粹的I/O操作，而不涉及复杂的计算。使用事件驱动来构建反向代理是更好的选择，一个工作进程即可运行，无需管理进程和线程带来的开销，同时CPU和内存消耗也较小。

因此，Nginx和Squid等服务器都是采用这种方式实现的。当然，Nginx也可以采用多进程加事件驱动的模式，几个进程运行libevent，而无需像Apache那样需要数百个进程。Nginx在处理静态文件时也表现出色，这是因为静态文件本身也属于磁盘I/O操作，处理方式相同。至于所谓的数万并发连接，这并没有多大意义。随手编写一个网络程序就可以处理数万个并发连接，但如果大多数客户端都被阻塞在某处，那就没有多少价值了。

再来看看像Apache或Resin这样的应用服务器，它们之所以被称为应用服务器，是因为它们需要运行具体的业务应用程序，如科学计算、图形图像处理、数据库读写等。它们很可能是CPU密集型的服务，而事件驱动并不适合此类情况。举个例子，如果某项计算需要2秒的耗时，那么这2秒将完全阻塞进程，事件机制毫无作用。想象一下，如果MySQL改用事件驱动，一个大型的join或sort操作将阻塞所有客户端。在这种情况下，多进程或多线程展现出优势，每个进程可以独立地执行任务，彼此不会阻塞或干扰。当然，现代CPU速度越来越快，单个计算的阻塞时间可能很短，但只要存在阻塞，事件编程就不具备优势。因此，进程和线程等技术不会消失，而是与事件机制相辅相成，并将长期存在。

总而言之，事件驱动适用于I/O密集型服务，而多进程或多线程适用于CPU密集型服务。它们各自具有优势，并不存在取代彼此的趋势。

**本段文字参考：**

**————————————————**
**版权声明：本文为CSDN博主「席飞剑」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。**
**原文链接：https://blog.csdn.net/xifeijian/article/details/17385831**
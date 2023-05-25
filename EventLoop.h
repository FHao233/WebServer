//
// Created by chao on 23-5-25.
//

#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H


#include <functional>
#include <memory>
#include "Channel.h"

class EventLoop {

    // 初始化poller, event_fd，给 event_fd 注册到 epoll 中并注册其事件处理回调
    typedef std::function<void()> Function;

    EventLoop();

    ~EventLoop();

    // 开始事件循环 调⽤该函数的线程必须是该 EventLoop 所在线程，也就是 Loop 函数不能跨线程调⽤
    void Loop();

    //停止循环
    void StopLoop();

    //如果当前线程就是创建此EventLoop的线程 就调⽤callback(关闭连接 EpollDel) 否则就放⼊等待执⾏函数区
    void RunInLoop(Function &&func);

    // 把此函数放⼊等待执⾏函数区 如果当前是跨线程 或者正在调⽤等待的函数则唤醒
    void QueueInLoop(Function &&func);

    // 把fd和绑定的事件注册到epoll内核事件表
    void PollerAdd(std::shared_ptr<Channel> channel, int timeout = 0);
// 在epoll内核事件表修改fd所绑定的事件
    void PollerMod(std::shared_ptr<Channel> channel, int timeout = 0);
// 从epoll内核事件表中删除fd及其绑定的事件
    void PollerDel(std::shared_ptr<Channel> channel);
// 只关闭连接(此时还可以把缓冲区数据写完再关闭)
    void ShutDown(std::shared_ptr<Channel> channel);
    bool is_in_loop_thread();

private:
    // 创建eventfd 类似管道的 进程间通信⽅式
    static int CreateEventfd();

    void HandleRead(); // eventfd的读回调函数(因为event_fd写了数据，所以触发可读事件，从event_fd读数据)
    void HandleUpdate();// eventfd的更新事件回调函数(更新监听事件
    void WakeUp(); // 异步唤醒SubLoop的epoll_wait(向event_fd中写⼊数据)
    void PerformPendingFunctions(); // 执⾏正在等待的函数(SubLoop注册EpollAdd连接套接字以及绑定事件的函数)
private:
    std::shared_ptr<Poller> poller_; // io多路复⽤ 分发器
    int event_fd_; //⽤于异步唤醒 SubLoop 的 Loop 函数中的 Poll(epoll_wait因为还没有注册fd会⼀直阻塞)
    std::shared_ptr<Channel> wakeup_channel_; // ⽤于异步唤醒的 channel
    pid_t thread_id_; // 线程id
    mutable locker::MutexLock mutex_;
    std::vector<Function> pending_functions_; // 正在等待处理的函数
    bool is_stop_; // 是否停⽌事件循环
    bool is_looping_; // 是否正在事件循环
    bool is_event_handling_; // 是否正在处理事件
    bool is_calling_pending_functions_; // 是否正在调⽤等待处理的函数
};


#endif //WEBSERVER_EVENTLOOP_H

//
// Created by chao on 23-5-25.
//

#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H

#include <functional>
#include <memory>
#include "HttpData.h"
#include "EventLoop.h"

class Channel {
public:
    typedef std::function<void()> EventCallBack;

    Channel();

    explicit Channel(int fd);
    Channel(EventLoop *loop);
    Channel(EventLoop* loop, int fd);    //构造函数


    ~Channel();

    // IO事件回调函数的调⽤接⼝，EventLoop中调⽤Loop开始事件循环 会调⽤Poll得到就绪事件，然后依次调⽤此函数处理就绪事件
    void HandleEvents();

    void HandleRead(); // 处理读事件的回调
    void HandleWrite(); // 处理写事件的回调
    void HandleUpdate(); // 处理更新事件的回调
    void HandleError(); // 处理错误事件的回调
    int get_fd() { return fd_; }

    void set_fd(int fd) { fd_ = fd; }

    // 返回weak_ptr所指向的shared_ptr对象
//    std::shared_ptr<http::HttpConnection> holder();
    std::weak_ptr<HttpData> holder_;

    void set_holder(std::shared_ptr<HttpData> holder) { holder_ = holder; }

    std::shared_ptr<HttpData> getHolder() {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

    // 设置回调函数
    void set_read_handler(EventCallBack &&read_handler) { read_handler_ = read_handler; }

    void set_write_handler(EventCallBack &&write_handler) { write_handler_ = write_handler; }

    void set_update_handler(EventCallBack &&update_handler) { update_handler_ = update_handler; }

    void set_error_handler(EventCallBack &&error_handler) { error_handler_ = error_handler; }

    void set_revents(int revents) { revents_ = revents; }

    int &events();

    void set_events(int events) { events_ = events; }

    int last_events();

    bool update_last_events();

private:
    int fd_; // Channel的fd
    int events_; // Channel正在监听的事件（或者说感兴趣的事件
    int revents_; // 返回的就绪事件
    int last_events_; // 上⼀此事件（主要⽤于记录如果本次事件和上次事件⼀样 就没必要调⽤epoll_mod）
    EventLoop *loop_;

    EventCallBack read_handler_;
    EventCallBack write_handler_;
    EventCallBack update_handler_;
    EventCallBack error_handler_;
};

typedef std::shared_ptr<Channel> SP_Channel;
#endif //WEBSERVER_CHANNEL_H

//
// Created by chao on 23-5-25.
//

// #ifndef WEBSERVER_CHANNEL_H
// #define WEBSERVER_CHANNEL_H
#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "base/Timer.h"
/**
 *  Channel 就是对 fd 事件的封装，包括注册它的事件以及回调。 EventLoop 通过调⽤
Channel::handleEvent() 来执⾏ Channel 的读写事件。 Channel::handleEvent()
的实现也⾮常简单，就 是⽐较已经发⽣的事件（由 Poller
返回），来调⽤对应的回调函数（读、写、错误）。
*/
class EventLoop;
class HttpData;
class Channel {
 public:
  typedef std::function<void()> EventCallBack;

  // Channel();

  explicit Channel(int fd);
  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);  //构造函数

  ~Channel();

  // IO事件回调函数的调⽤接⼝，EventLoop中调⽤Loop开始事件循环
  // 会调⽤Poll得到就绪事件，然后依次调⽤此函数处理就绪事件
  void HandleEvents();

  void HandleRead();    // 处理读事件的回调
  void HandleWrite();   // 处理写事件的回调
  // void HandleUpdate();  // 处理更新事件的回调
  void HandleConn();
  void HandleError();  // 处理错误事件的回调
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
  void set_read_handler(EventCallBack &&read_handler) {
    read_handler_ = read_handler;
  }

  void set_write_handler(EventCallBack &&write_handler) {
    write_handler_ = write_handler;
  }

  void set_update_handler(EventCallBack &&update_handler) {
    update_handler_ = update_handler;
  }

  void set_error_handler(EventCallBack &&error_handler) {
    error_handler_ = error_handler;
  }

  void set_conn_handler(EventCallBack &&conn_handler) {
    conn_handler_ = conn_handler;
  }

  void set_revents(__uint32_t revents) { revents_ = revents; }

  // int &events();

  void set_events(__uint32_t events) { events_ = events; }
  __uint32_t &getEvents() { return events_; }

  // int last_events();
  // bool update_last_events();

  __uint32_t getLastEvents() { return last_events_; }
  bool EqualAndUpdateLastEvents() {
    bool ret = (last_events_ == events_);
    last_events_ = events_;
    return ret;
  }

 private:
  int fd_;             // Channel的fd
  __uint32_t events_;  // Channel正在监听的事件（或者说感兴趣的事件
  __uint32_t revents_;  // 返回的就绪事件
  __uint32_t
      last_events_;  // 上⼀此事件（主要⽤于记录如果本次事件和上次事件⼀样
                     // 就没必要调⽤epoll_mod）
  EventLoop *loop_;

  EventCallBack read_handler_;
  EventCallBack write_handler_;
  EventCallBack update_handler_;
  EventCallBack error_handler_;
  EventCallBack conn_handler_;
};
typedef std::shared_ptr<Channel> SP_Channel;

// #endif  // WEBSERVER_CHANNEL_H

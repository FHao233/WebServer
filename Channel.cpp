//
// Created by chao on 23-5-25.
//

#include "Channel.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include <queue>

#include "Epoll.h"
#include "EventLoop.h"
#include "Util.h"

Channel::Channel(EventLoop *loop)
    : loop_(loop), events_(0), last_events_(0), fd_(0) {}
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), last_events_(0) {}
Channel::~Channel() {
  // loop_->poller_->epoll_del(fd, events_);
  // close(fd_);
}

void Channel::HandleEvents()  //这段代码实现了 Channel 类的事件处理函数
                              //HandleEvents()，用于处理 epoll 监听到的事件。
{
  events_ = 0;  //首先将 events_ 置为 0，然后根据 revents_ 的值进行不同的处理：
  if ((revents_ & EPOLLHUP) &&
      !(revents_ & EPOLLIN)) {  //如果 revents_ 包含 EPOLLHUP 且不包含
                                //EPOLLIN，则将 events_ 置为 0 并返回；
    events_ = 0;
    return;
  }
  if (revents_ & EPOLLERR) {  //如果 revents_ 包含 EPOLLERR，则调用错误处理函数
                              //error_handler_()，将 events_ 置为 0 并返回；
    if (error_handler_) error_handler_();
    events_ = 0;
    return;
  }
  if (revents_ & (EPOLLIN | EPOLLPRI |
                  EPOLLRDHUP)) {  //如果 revents_ 包含 EPOLLIN、EPOLLPRI 或
                                  //EPOLLRDHUP，则调用 HandleRead() 处理读事件；
    HandleRead();
  }
  if (revents_ & EPOLLOUT) {  //如果 revents_ 包含 EPOLLOUT，则调用
                              //HandleWrite() 处理写事件；
    HandleWrite();
  }
  HandleConn();  // 最后调用 HandleConn() 处理连接事件。
}
void Channel::HandleRead() {
  if (read_handler_) {
    read_handler_();
  }
}

void Channel::HandleWrite() {
  if (write_handler_) {
    write_handler_();
  }
}

void Channel::HandleConn() {
  if (conn_handler_) {
    conn_handler_();
  }
}
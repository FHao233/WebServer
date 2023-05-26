//
// Created by chao on 23-5-25.
//
#include "EventLoop.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "Epoll.h"
#include "Util.h"
__thread EventLoop *t_loopInThisThread = 0;
auto EventLoop::CreateEventfd() -> int {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    //        LOG << "Failed in eventfd";
    //        abort();
  }
  return evtfd;
}
/**
 * 这代码的作用是在事件循环中处理读事件，并且使用边缘触发模式进行事件监听。当有数据可读时，会触发事件环中的相应回调函数进行处理。
 */
void EventLoop::HandleRead() {
  uint64_t one = 1;                                //占用8个字节
  ssize_t n = readn(event_fd_, &one, sizeof one);  //每次读取8个字节
  wakeup_channel_->set_events(
      EPOLLIN |
      EPOLLET);  //将wakeup_channel_的事件类型设置为EPOLLIN |
                 //EPOLLET，表示该通道已经准备好进行读取操作，并且使用边缘触发模式(EPOLLET)进行事件监听。
}
EventLoop::EventLoop()
    : is_looping_(false),
      poller_(new Epoll()),
      event_fd_(CreateEventfd()),
      is_event_handling_(false),
      is_calling_pending_functions_(false),
      thread_id_(CurrentThread::tid()),
      wakeup_channel_(new Channel(this, event_fd_)) {
  if (t_loopInThisThread) {
  } else {
    t_loopInThisThread = this;
  }
  wakeup_channel_->set_events(EPOLLIN | EPOLLET);
  wakeup_channel_->set_read_handler(bind(&EventLoop::handleRead, this));
}

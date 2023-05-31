//
// Created by chao on 23-5-25.
//
#include "EventLoop.h"

#include <assert.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <iostream>

#include "Util.h"
#include "base/Logger.h"

__thread EventLoop *t_loopInThisThread =
    0;  // 用于存储当前线程所属的EventLoop对象指针。

bool EventLoop::is_in_loop_thread()
    const {  //判断当前线程是否为EventLoop所在的线程。如果是，返回true，否则返回false。
  return thread_id_ == CurrentThread::tid();
}

auto EventLoop::CreateEventfd() -> int {
  int evtfd = eventfd(
      0,
      EFD_NONBLOCK |
          EFD_CLOEXEC);  // eventfd是Linux内核提供的一种事件通知机制，可以用于实现异步事件处理。
                         // 它被用于实现EventLoop的事件循环机制。
  // E_NONBLOCK和EFD_CLOEXEC是eventfd的两个选项，分别表示非阻塞模式和执行exec函数时关闭文件描述符。
  if (evtfd < 0) {
    LOG << "Failed in eventfd";
    abort();
  }
  return evtfd;
}
/**
 * 这代码的作用是在事件循环中处理读事件，并且使用边缘触发模式进行事件监听。当有数据可读时，会触发事件环中的相应回调函数进行处理。
 */
void EventLoop::HandleRead() {
  uint64_t one = 1;                                //占用8个字节
  ssize_t n = readn(event_fd_, &one, sizeof one);  //每次读取8个字节
  if (n != sizeof one) {
    LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  wakeup_channel_->set_events(
      EPOLLIN |
      EPOLLET);  //将wakeup_channel_的事件类型设置为EPOLLIN |
                 // EPOLLET，表示该通道已经准备好进行读取操作，并且使用边缘触发模式(EPOLLET)进行事件监听。
}
// 是唤醒EventLoop的事件循环机制
void EventLoop::WakeUp() {
  uint64_t one = 1;
  ssize_t n = writen(event_fd_, (char *)(&one), sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}
// 在EventLoop中，待处理函数通常是由其他线程通过QueueInLoop函数添加的。
// 当EventLoop检测到有待处理函数时，会调用doPendingFunctors函数执行这些函数。
// 由于待处理函数可能会修改EventLoop中的状态，因此需要使用互斥锁保护它们的执行。
void EventLoop::PerformPendingFunctions() {
  std::vector<Function> functions;
  is_calling_pending_functions_ = true;
  {
    MutexLockGuard lock(mutex_);
    functions.swap(pending_functions_);
  }
  for (size_t i = 0; i < functions.size(); ++i) functions[i]();
  is_calling_pending_functions_ = false;
}
EventLoop::EventLoop()
    : is_looping_(false),
      poller_(new Epoll()),
      event_fd_(CreateEventfd()),
      is_stop_(false),
      is_event_handling_(false),
      is_calling_pending_functions_(false),
      thread_id_(CurrentThread::tid()),
      wakeup_channel_(new Channel(this, event_fd_)) {
  if (t_loopInThisThread) {
  } else {
    t_loopInThisThread = this;
  }
  wakeup_channel_->set_events(EPOLLIN | EPOLLET);
  wakeup_channel_->set_read_handler(std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->set_conn_handler(std::bind(&EventLoop::HandleConn, this));
  poller_->epoll_add(wakeup_channel_, 0);
}
EventLoop::~EventLoop() {
  close(event_fd_);
  t_loopInThisThread = NULL;
}
void EventLoop::HandleConn() { PollerMod(wakeup_channel_, 0); }
void EventLoop::Loop() {
  assert(!is_looping_);         // 确保EventLoop没有处于事件循环状态
  assert(is_in_loop_thread());  // 确保当前线程是EventLoop所在的线程
  is_looping_ = true;           // 标记EventLoop正在事件循环中
  is_stop_ = false;             // 标记EventLoop没有被请求停止
  std::vector<SP_Channel> ret;  // 定义一个存储Channel指针的向量ret
  while (!is_stop_) {           // 循环直到EventLoop被请求停止
    ret.clear();                // 清空ret
    ret = poller_->poll();  // 调用poller_的poll函数获取当前可读/可写的Channel
    is_event_handling_ = true;  // 标记EventLoop正在处理事件
    for (auto &it : ret)
      it->HandleEvents();  // 遍历ret中的每个Channel，调用其HandleEvents函数处理事件
    is_event_handling_ = false;  // 标记EventLoop事件处理完毕
    PerformPendingFunctions();   // 执行待处理函数
    poller_->handleExpired();    // 处理超时的定时器
  }
  is_looping_ = false;
}
// 当其他线程调用quit函数请求退出事件循环时，如果当前线程不是EventLoop所在的线程，
// 那么EventLoop可能正在阻塞等待事件的到来，此时需要通过wakeup函数唤醒它，使其能够及时响应退出请求，停止事件循环并退出。
void EventLoop::StopLoop() {
  is_stop_ = true;
  if (!is_in_loop_thread()) {
    WakeUp();
  }
}

void EventLoop::RunInLoop(Function &&func) {
  if (is_in_loop_thread()) {
    func();
  } else {
    QueueInLoop(std::move(func));
  }
}

void EventLoop::QueueInLoop(Function &&func) {
  {
    MutexLockGuard lock(mutex_);
    pending_functions_.emplace_back(std::move(func));
  }
  if (!is_in_loop_thread() || is_calling_pending_functions_) WakeUp();
}

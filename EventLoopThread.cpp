#include "EventLoopThread.h"

#include <assert.h>

#include <functional>
EventLoopThread::EventLoopThread()
    : loop_(NULL), // 初始化loop_指针为NULL
      exiting_(false),// 初始化exiting_为false
      thread_(std::bind(&EventLoopThread::threadFunc, this), "EventLoopThread"), // 初始化thread_，绑定线程函数为threadFunc，线程名为EventLoopThread
      mutex_(),// 初始化互斥锁mutex_
      cond_(mutex_) {}// 初始化条件变量cond_，并将其与mutex_关联

EventLoopThread::~EventLoopThread() {
  exiting_ = true;// 设置exiting_为true，表示线程退出
  if (loop_ != NULL) { // 如果loop_指针不为NULL
    loop_->StopLoop();// 停止EventLoop循环
    thread_.join(); // 等待线程结束
  }
}

EventLoop *EventLoopThread::startLoop() {
  assert(!thread_.started()); // 断言线程未启动
  thread_.start();// 启动线程
  {
    MutexLockGuard lock(mutex_); // 加锁
    // 一直等到threadFun在Thread里真正跑起来
    while (loop_ == NULL) cond_.wait();// 等待loop_指针被赋值
  }
  return loop_;// 返回EventLoop指针
}
void EventLoopThread::threadFunc() {
  EventLoop loop;// 创建EventLoop对象
  {
    MutexLockGuard lock(mutex_); // 加锁
    loop_ = &loop; // 将loop_指针指向新创建的EventLoop对象
    cond_.notify();// 通知等待的线程
  }
  loop.Loop(); // 运行EventLoop循环
  loop_ = NULL;// 循环结束，将loop_指针置为NULL
}

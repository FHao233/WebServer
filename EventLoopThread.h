#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "EventLoop.h"
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
//定义了一个名为EventLoopThread的类，它的作用是创建一个新线程并在该线程中运行一个EventLoop事件循环
//在startLoop()函数中，首先使用mutex_加锁，然后创建一个新的线程，并将threadFunc()函数作为线程的执行函数。
//在threadFunc函数中，首先创建一个EventLoop，并将其保存在loop_成员变量中。然后使用cond_等待条件变量，直到exiting_成员变量为true，表示线程需要退出。
//最后，销毁EventLoop对象，并将_成员变量置为NULL。
//通过这个类，可以方便地创建一个新的线程，并在该线程中运行一个EventLoop事件循环，从实现多线程编程。
class EventLoopThread : noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();  // startLoop()方法会启动一个新线程，并在该线程中创建一个EventLoop对象，然后返回该对象的指针。

 private:
  void threadFunc();  //该类的threadFunc()方法是新线程的入口函数，它会在新线程中运行EventLoop循环。
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};
#endif
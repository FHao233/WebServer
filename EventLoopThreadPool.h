#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <memory>
#include <vector>

#include "EventLoopThread.h"
#include "base/Logger.h"
#include "base/noncopyable.h"
// 这段代码定义了一个名为EventLoopThreadPool的类，它的作用是创建一个包含多个EventLoop对象的线程池，以便在多个线程中同时运行事件循环。
class EventLoopThreadPool : noncopyable {
  EventLoopThreadPool(EventLoop* baseLoop, int numThreads); //需要传入基础的EventLoop对象和一个整数值，表示需要创建的线程数。

  ~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; }
  void start();// 启动所有线程，并在每个线程中创建一个EventLoop对象。

  EventLoop* getNextLoop();// 返回下一个可用的EventLoop对象，以便在该对象上运行事件循环。

 private:
  EventLoop* baseLoop_; // 基础的EventLoop对象
  bool started_;// 线程池是否已经启动
  int numThreads_; // 线程数
  int next_;// 下一个可用的EventLoop对象的索引
  std::vector<std::shared_ptr<EventLoopThread>> threads_; // 线程池中的线程
  std::vector<EventLoop*> loops_;// 线程池中的EventLoop对象
};

#endif
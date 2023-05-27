/**
 * 这段代码定义了一个 Thread 类，用于创建线程。该类包含了以下主要功能：
      可以传入一个函数指针作为线程函数，用于在线程中执行特定的任务。 
      可以设置线程名，方便调试和日志记录。
      可以启动线程，等待线程结束并回收资源。 
      可以获取线程 ID 和线程名。
      可以判断线程是否已经启动。
    该类的实现依赖于 CountDownLatch 类和 nonable 类，其中 CountDownLatch 类用于实现倒计时门闩，noncopyable 类用于禁止拷贝和赋值操作，以保证线程安全。
*/
#ifndef WEBSERVER_THREAD_H
#define WEBSERVER_THREAD_H

#include <functional>
#include <string>
#include "CountDownLatch.h"
#include "noncopyable.h"
#include <string>

class Thread : noncopyable {
 public:
  typedef std::function<void()> ThreadFunc; // 定义 ThreadFunc 类型为函数指针
  explicit Thread(const ThreadFunc&, const std::string& name = std::string()); // 构造，传入 ThreadFunc 和线程名 name
  ~Thread();// 析构函数
  void start();// 启动线程
  int join();// 等待线程结束并回收资源
  bool started() const { return started_; }// 返回线程是否已经启动
  pid_t tid() const { return tid_; }// 返回线程 ID
  const std::string& name() const { return name_; }// 返回线程名

 private:
  void setDefaultName();// 设置默认线程名
  bool started_;// 标记线程是否已经启动
  bool joined_;// 标记线程是否经结束并回收资源
  pthread_t pthreadId_;// 线程 ID
  pid_t tid_;// 线程 ID
  ThreadFunc func_;// 线程函数
  std::string name_;// 线程名
  CountDownLatch latch_;// 倒计时门闩，用于等待线程启动
};

#endif  // WEBSERVER_THREAD_H

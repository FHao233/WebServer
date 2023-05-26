#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "CountDownLatch.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "Thread.h"
#include "noncopyable.h"
/**
 * 这个类的作⽤则是将从前端获得的 Buffer A 放⼊ 后端的 Buffer B中，并且将 Buffer B
    的内容最终写⼊到磁盘中,
*/
class AsyncLogging : noncopyable {
 public:
  AsyncLogging(const std::string basename, int flushInterval = 2);// 构造函数
  ~AsyncLogging() {// 析构函数
    if (running_) stop();
  }
  void append(const char* logline, int len);// 添加日志

  void start() { // 启动异步日志线程
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop() { // 停止异步日志线程
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:
  void threadFunc();// 日志线程函数,后端⽇志线程函数，⽤于把缓冲区⽇志写⼊⽂件
  typedef FixedBuffer<kLargeBuffer> Buffer;// 缓冲区类型
  typedef std::vector<std::shared_ptr<Buffer>> BufferVector;// 缓冲区指针向量类型
  typedef std::shared_ptr<Buffer> BufferPtr; // 缓冲区指针类型
  const int flushInterval_;// 超时时间，每隔⼀段时间写⽇志
  bool running_;// 日志线程是否在运行
  std::string basename_; // 日志文件名
  Thread thread_;// 后端线程，⽤于将⽇志写⼊⽂件
  MutexLock mutex_; // 互斥锁
  Condition cond_;// 条件变量
  BufferPtr currentBuffer_;// 当前缓冲区
  BufferPtr nextBuffer_;// 下一个缓冲区 双缓冲区方法，减少前端等待的开销
  BufferVector buffers_;// 缓冲区指针向量 缓冲区队列，实际写⼊⽂件的内容也是从这⾥拿的。
  CountDownLatch latch_;// 倒计时，⽤于指示⽇志记录器何时开始⼯作
};

#endif
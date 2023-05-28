#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_(baseLoop), started_(false), numThreads_(numThreads), next_(0) {
  if (numThreads_ <= 0) {  // 如果线程数小于等于0，输出错误信息并终止程序
    LOG << "numThreads_ <= 0";
    abort();
  }
}

void EventLoopThreadPool::start() {
  baseLoop_
      ->assertInLoopThread();  // 确保在baseLoop_所在的线程中调用start()方法
  started_ = true;             // 标记线程池已经启动
  for (int i = 0; i < numThreads_; ++i) {
    std::shared_ptr<EventLoopThread> t(
        new EventLoopThread());  // 创建numThreads_个线程，并在每个线程中创建一个EventLoop对象
    threads_.push_back(t);
    loops_.push_back(t->startLoop()); // 开始循环
  }
}
EventLoop *EventLoopThreadPool::getNextLoop() {
  baseLoop_
      ->assertInLoopThread();  // 确保在baseLoop_所在的线程中调用getNextLoop()方法
  assert(started_);             // 确保线程池已经启动
  EventLoop *loop = baseLoop_;  // 默认返回baseLoop_对象
  if (!loops_.empty()) {        // 如果线程池中有EventLoop对象
    loop = loops_[next_];       // 返回一个可用的EventLoop对象
    next_ = (next_ + 1) % numThreads_;  // 更新下一个可用的EventLoop对象的索引
  }
  return loop;
}
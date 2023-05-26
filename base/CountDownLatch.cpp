#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    : mutex_(), condition_(mutex_), count_(count) {}// 构造函数，初始化计数器、互斥锁和条件变量

void CountDownLatch::wait() {
  MutexLockGuard lock(mutex_);// 加锁
  while (count_ > 0) condition_.wait();// 等待计数器归零
}

void CountDownLatch::countDown() {
  MutexLockGuard lock(mutex_);// 加锁
  --count_;// 计数器减一
  if (count_ == 0) condition_.notifyAll();// 如果计数器归零，唤醒所有等待线程
}

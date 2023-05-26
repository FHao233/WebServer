#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H
#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"
// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后
// 外层的start才返回
class CountDownLatch : noncopyable {
 public:
  explicit CountDownLatch(int count);
  void wait();// 等待计数器归零
  void countDown(); // 计数器减一

 private:
  mutable MutexLock mutex_; // 可变的互斥锁对象
  Condition condition_;// 条件变量对象
  int count_;// 计数器
};

#endif
#ifndef CONDITION_H
#define CONDITION_H
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "MutexLock.h"
#include "noncopyable.h"
class Condition : noncopyable {
 public:
  explicit Condition(MutexLock &_mutex)
      : mutex(_mutex) {  // 构造函数，初始化条件变量和互斥锁
    pthread_cond_init(&cond, NULL);
  }
  ~Condition() { pthread_cond_destroy(&cond); }  // 析构函数，销毁条件变量
  void wait() { pthread_cond_wait(&cond, mutex.get()); }  // 等待条件变量
  void notify() { pthread_cond_signal(&cond); }  // 唤醒一个等待条件变量的线程
  void notifyAll() {
    pthread_cond_broadcast(&cond);
  }  // 唤醒所有等待条件变量的线程
  bool waitForSeconds(
      int seconds) {  // 等待一段时间，如果条件变量没有被唤醒，则返回false，否则返回true
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);         // 获取当前时间
    abstime.tv_sec += static_cast<time_t>(seconds);  // 计算超时时间
    return ETIMEDOUT == pthread_cond_timedwait(
                            &cond, mutex.get(),
                            &abstime);  // 等待条件变量，如果超时则返回false
  }

 private:
  MutexLock &mutex;
  pthread_cond_t cond;
};

#endif
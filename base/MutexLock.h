#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H
#include <pthread.h>  // pthread 头文件,用于操作互斥锁

#include "noncopyable.h"  // 包含 noncopyable 类,禁止拷贝构造和赋值函数
class MutexLock : noncopyable {  // MutexLock 类,继承自 noncopyable,禁止拷贝
 public:
  MutexLock() { pthread_mutex_init(&mutex, NULL); }  // 构造函数,初始化互斥锁
  ~MutexLock() {
    pthread_mutex_lock(
        &mutex);  // 析构函数,先锁定互斥锁
                  // 通过在析构函数中先锁定互斥锁，我们确保所有其他线程在销毁互斥锁之前都已经释放了该锁。这可以保证在销毁互斥锁时没有其他线程正在使用该锁，从而避免任何潜在的同步问题。
    pthread_mutex_destroy(&mutex);  // 然后销毁互斥锁
  }
  void lock() { pthread_mutex_lock(&mutex); }      // 锁定互斥锁
  void unlock() { pthread_mutex_unlock(&mutex); }  // 解锁互斥锁
  pthread_mutex_t *get() { return &mutex; }        // 获取互斥锁指针

 private:
  pthread_mutex_t mutex;  // 互斥锁变量
                          // 友元类不受访问权限影响
 private:
  friend class Condition;
};
class MutexLockGuard
    : noncopyable {  // MutexLockGuard 类,用于自动锁定和解锁互斥锁
 public:
  explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) {
    mutex.lock();
  }                                      // 构造时锁定互斥锁
  ~MutexLockGuard() { mutex.unlock(); }  // 析构时解锁互斥锁

 private:
  MutexLock &mutex;  // 所管理的互斥锁引用
};

#endif
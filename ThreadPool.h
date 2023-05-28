#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H
#include <pthread.h>

#include <functional>
#include <memory>
#include <vector>

#include "Channel.h"
const int THREADPOOL_INVALID = -1;         // 线程池无效
const int THREADPOOL_LOCK_FAILURE = -2;    // 线程池锁失败
const int THREADPOOL_QUEUE_FULL = -3;      // 线程池队列已满
const int THREADPOOL_SHUTDOWN = -4;        // 线程池已关闭
const int THREADPOOL_THREAD_FAILURE = -5;  // 线程池线程失败
const int THREADPOOL_GRACEFUL = 1;         // 线程池优雅关闭

const int MAX_THREADS = 1024;  //最大线程数
const int MAX_QUEUE = 65535;   // 最大队列长度
typedef enum { immediate_shutdown = 1, graceful_shutdown = 2 } ShutDownOption;
struct ThreadPoolTask {
  std::function<void(std::shared_ptr<void>)> fun;  // 任务函数
  std::shared_ptr<void> args;                      // 任务参数
};
class ThreadPool {
 private:
  static pthread_mutex_t lock;               // 线程池锁
  static pthread_cond_t notify;              // 线程池条件变量
  static std::vector<pthread_t> threads;     // 线程池中的线程
  static std::vector<ThreadPoolTask> queue;  // 线池任务队列

  static int thread_count;  // 线程数
  static int queue_size;    // 队列长度
  static int head;          // 队列头指针
  // tail 指向尾节点的下一节点
  static int tail;      // 队列尾指针
  static int count;     // 队列中的任务数
  static int shutdown;  // 线程池是否关闭
  static int started;   // 线程池是否启动

 public:
  static int threadpool_create(int _thread_count,
                               int _queue_size);  // 创建线程池
  static int threadpool_add(
      std::shared_ptr<void> args,
      std::function<void(std::shared_ptr<void>)> fun);  // 添加任务到线程池
  static int threadpool_destroy(
      ShutDownOption shutdown_option = graceful_shutdown);  // 销毁线程池
  static int threadpool_free();                // 释放线程池资源
  static void *threadpool_thread(void *args);  // 线程池中的线程函数
};

#endif
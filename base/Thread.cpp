//
// Created by chao on 23-5-25.
//
#include "Thread.h"

#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "CurrentThread.h"
namespace CurrentThread {  // 命名空间 CurrentThread，用于存储当前线程的相关信息
__thread int t_cachedTid = 0;  // 线程 ID 的缓存，使用 __thread 关键字保证程安全
__thread char t_tidString[32];  // 线程 ID 的字符串形式，用于日志输出等
__thread int t_tidStringLength = 6;             // 线程 ID 字符串的长度
__thread const char* t_threadName = "default";  // 线程名，默认为 "default"
}  // namespace CurrentThread
pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}  // 获取线程 ID 的系统调用函数调用系统调用 gettid() 获取当前线程的线程
   // ID。::syscall()是一个系统调用函数，SYS_gettid 是一个宏定义，表示获取线程
   // ID 的系统调用编号。
void CurrentThread::cacheTid() {  // 缓存线程 ID
  if (t_cachedTid == 0) {  // 如果线程 ID 还未被缓存，则获取线程 ID 并缓存
    t_cachedTid = gettid();
    t_tidStringLength = snprintf(
        t_tidString, sizeof t_tidString, "%5d ",
        t_cachedTid);  // 将线程 ID 转换为字符串形式并存储到 t_tidString 中
  }
}

// 为了在线程中保留name,tid这些数据
struct ThreadData {
  // 定义 ThreadFunc 类型，ThreadFunc
  // 是一个函数指针类型，指向一个无返回值、无参数的函数  typedef
  typedef Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;  // ThreadFunc 类型的成员变量，用于保存程函数
  std::string name_;       // 线程名称
  pid_t* tid_;             // 线程 ID
  CountDownLatch* latch_;  // 用于线程同步的计数器
  ThreadData(const ThreadFunc& func, const std::string& name, pid_t* tid,
             CountDownLatch* latch)  // 构造函数，用于初始化成员变量
      : func_(func), name_(name), tid_(tid), latch_(latch) {}
  // 线程函数，用于执行线程任务
  void runInThread() {
    *tid_ = CurrentThread::tid();  // 获取当前线程的线程 ID，并将其保存到 tid_
                                   // 指向的变量中
    tid_ = NULL;
    latch_->countDown();  // 计数器减一，用于线程同步
    latch_ = NULL;
    // 设置当前线程的名称
    CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
    prctl(
        PR_SET_NAME,
        CurrentThread::
            t_threadName);  // 设置线程名称，方便调试 prctl()
                            // 是一个系统调用，用于控制进程的运行时行为。在这里，PR_SET_NAME
                            // 是 prctl
                            // 的一个选项，用于设置进程的名称。CurrentThread::t_threadName是一个静态变量，用于保存当前线程的名称。通过调用

    func_();  // 执行线程函数
    CurrentThread::t_threadName =
        "finished";  // 线程执行完毕，将线程名称设置为 "finished"
  }
};

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : started_(false),  // 初始化 started_ 为 false
      joined_(false),   // 初始化 joined_ 为 false
      pthreadId_(0),    // 初始化 pthreadId_ 为 0
      tid_(0),          // 初始化 tid_ 为 0
      func_(func),      // 初始化 func_ 为传入的函数对象
      name_(name),      // 初始化 name_ 为传的线程名称
      latch_(1) {       // 初始化计数器为 1
  setDefaultName();     // 设置默认线程名称
}
void Thread::setDefaultName() {
  if (name_.empty()) {  // 如果线程名称为空
    char buf[32];
    snprintf(buf, sizeof buf, "Thread");  // 生成默认线程名称
    name_ = buf;  // 将默认线程名称赋值给 name_
  }
}

Thread::~Thread() {
  if (started_ && !joined_)
    pthread_detach(pthreadId_);  // 如果线程已经启动但未被 join，则将其分离
}
void* startThread(void* obj) {
  ThreadData* data =
      static_cast<ThreadData*>(obj);  // 将传入的参数转换为 ThreadData型
  data->runInThread();                // 在新线程中执行线程函数
  delete data;                        // 释放 ThreadData 对象
  return NULL;
}

void Thread::start() {
  started_ = true;  // 标记线程已经启动
  ThreadData* data =
      new ThreadData(func_, name_, &tid_, &latch_);  // 创建 ThreadData 对象
  if (pthread_create(
          &pthreadId_, NULL, &startThread,
          data)) {  // 创建新线程
                    // &startThread用于指定线程的入口函数，即新线程启动后要执行的函数。data：用于传递给线程入口函数的参数，类型为
                    // void*。
    started_ = false;  // 如果创建线程失败，则标记线程未启动
    delete data;       // 释放 ThreadData 对象
  } else {
    assert(tid_ > 0);
    latch_.wait();  // 等待新线程启动
  }
}

int Thread::join() {
  joined_ = true;  // 标记线程已经被 join
  return pthread_join(
      pthreadId_,
      NULL);  // 等待线程结束
              // 作用是等待指定的线程结束，并回收该线程的资源。具体来说，该函数的参数含义如下：
              // pthreadId_ 要等待的线程 ID，为 pthread_t 类型。
  // NULL：用于存储线程的返回值，如果不需要获取线程返回值，则可以将该参数设置为
  // NULL。
}

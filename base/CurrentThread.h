//
// Created by chao on 23-5-25.
//

#ifndef WEBSERVER_CURRENTTHREAD_H
#define WEBSERVER_CURRENTTHREAD_H
namespace CurrentThread {
// internal
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

void cacheTid();  // 声明获取当前线程的线程 ID 的函数

inline int tid() {
  if (__builtin_expect(
          t_cachedTid == 0,
          0)) {  //提示编译器 t_cachedTid == 0
                 //的执行概率很低，因此编译器会将这个分支放在代码的冷路径上，以避免对热路径的影响。这样可以提高代码的执行效率。
    cacheTid();
  }
  return t_cachedTid;
}

inline const char *tidString()  // for logging
{
  return t_tidString;
}

inline int tidStringLength()  // for logging
{
  return t_tidStringLength;
}

inline const char *name() { return t_threadName; }
}  // namespace CurrentThread
#endif  // WEBSERVER_CURRENTTHREAD_H

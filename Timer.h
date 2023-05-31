#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>

#include <deque>
#include <memory>
#include <queue>

#include "HttpData.h"
#include "base/MutexLock.h"
#include "base/noncopyable.h"

class HttpData;

/**
 * 这些代码实现了一个简单的定时器管理器，包括 TimerNode 和 TimerManager两个类。
 * TimerNode类表示一个定时器节点，包含了定时器的过期时间、关联的请求等信息，
 * 可以更新定时、判断定时器是否有效、清除定时器关联的请求等操作。
 * TimerManager类表示一个定时器管理器，包含了一个定时器队列，可以添加定时器、处理过期事件等操作。
 * 在添加定时器时，会创建一个TimerNode对象，并将其插入到定时器队列中；在处理过期事件时，会从定时器队列中取出已经过期的定时器，并执行其关联的请求。
   这些代码的作用是实现一个简单的定时器管理器，可以用于管理 HTTP请求的超时时间，防止请求长时间占用服务器资源。
 */
class TimerNode {
 public:
  TimerNode(std::shared_ptr<HttpData> requestData, int timeout);
  ~TimerNode();
  TimerNode(TimerNode &tn);                    // 拷贝构造函数
  void update(int timeout);                    // 更新定时器
  bool isValid();                              // 判断定时器是否有效
  void clearReq();                             // 清除定时器关联的请求
  void setDeleted() { deleted_ = true; }       // 标记定时已被删除
  bool isDeleted() const { return deleted_; }  // 判断定时器是否已被删除
  size_t getExpTime() const { return expiredTime_; }  // 获取定时器过期时间

 private:
  bool deleted_;                         // 标记定时器是否已被删除
  size_t expiredTime_;                   // 定时器过期时间
  std::shared_ptr<HttpData> SPHttpData;  // 定时器关联的请求
};

struct TimerCmp {
  bool operator()(std::shared_ptr<TimerNode> &a,
                  std::shared_ptr<TimerNode> &b) const {  // 定义定时器比较函数
    return a->getExpTime() > b->getExpTime();  // 按过期时间从小到大排序
  }
};

class TimerManager {
 public:
  TimerManager();
  ~TimerManager();
  void addTimer(std::shared_ptr<HttpData> SPHttpData,
                int timeout);  // 添加定时器
  void handleExpiredEvent();   // 处理过期事件

 private:
  typedef std::shared_ptr<TimerNode> SPTimerNode;  // 定义智能指针类型
  std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp>
      timerNodeQueue;  // 定时器队列
};

#endif
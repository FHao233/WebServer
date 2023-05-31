//
// Created by chao on 23-5-25.
//

#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H
#include <sys/epoll.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"
/**
 * Poller
类的作⽤就是负责监听⽂件描述符事件是否触发以及返回发⽣事件的⽂件描述符以及具体事件。所以⼀个
Poller 对象对应⼀个 IO 多路复⽤模块。在 muduo 中，⼀个 EventLoop 对应⼀个 Poller
。
*/

class Epoll {
 public:
  ~Epoll();

  void epoll_add(const SP_Channel &request, int timeout);

  void epoll_mod(const SP_Channel &request, int timeout);

  void add_timer(SP_Channel request_data, int timeout);

  void epoll_del(const SP_Channel &request);

  //   void poll(std::vector<SP_Channel> &req);
  std::vector<SP_Channel> poll();
  std::vector<SP_Channel> getEventsRequest(int events_num);
  int getEpollFd() { return epollFd_; }
  void handleExpired();
  Epoll();

 private:
  int epollFd_;  //就是⽤ epoll_create ⽅法返回的 epoll 句柄，这个是常识。
  static const int MAXFDS = 100000;//定义了服务器最大支持的文件描述符。
  std::vector<epoll_event> events_;//存放 epoll_wait()返回的活动事件（是⼀个结构体）
  std::shared_ptr<Channel> fd2chan_[MAXFDS];//一个数组，用于将文件描述符映射到对应的 Channel 对象，方便在事件处理函数中快速找到对应的 Channel 对象。
  std::shared_ptr<HttpData> fd2http_[MAXFDS];//一个数组，用于将文件描述符映射到对应的 HttpData 对象，方便在事件处理函数中快速找到应的 HttpData 对象。
  TimerManager timerManager_;//一个定时器管理器，用于管理所有的定时器，包括 HttpData 对象的定时器和 Channel 对象的定时器。
};

#endif  // WEBSERVER_EPOLL_H

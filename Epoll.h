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
#include "base/Timer.h"
/**
 * Poller 类的作⽤就是负责监听⽂件描述符事件是否触发以及返回发⽣事件的⽂件描述符以及具体事件。所以⼀个
Poller 对象对应⼀个 IO 多路复⽤模块。在 muduo 中，⼀个 EventLoop 对应⼀个 Poller 。
*/
class Epoll {
 public:
  ~Epoll();

  void epoll_add(const SP_Channel &request, int timeout);

  void epoll_mod(const SP_Channel &request, int timeout);

  void add_timer(SP_Channel request_data, int timeout);

  void epoll_del(const SP_Channel &request);

  //   void poll(std::vector<SP_Channel> &req);
  std::vector<std::shared_ptr<Channel>> poll();
  std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
  int getEpollFd() { return epollFd_; }
  void handleExpired();

 public:
  Epoll();

 private:
  int epollFd_;  //就是⽤ epoll_create ⽅法返回的 epoll 句柄，这个是常识。
                 //    std::vector<epoll_event> events_;//存放 epoll_wait()
                 //    返回的活动事件（是⼀个结构体）
  // std::unordered_map<int, SP_Channel> channelMap_;//负责记录⽂件描述符fd ->
  // Channel 的映射，也帮忙保管所有注册在你这个 Poller 上的 Channel
  static const int MAXFDS = 100000;
  std::vector<epoll_event> events_;
  std::shared_ptr<Channel> fd2chan_[MAXFDS];
  std::shared_ptr<HttpData> fd2http_[MAXFDS];
  TimerManager timerManager_;
};

#endif  // WEBSERVER_EPOLL_H

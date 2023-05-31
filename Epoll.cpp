
#include "Epoll.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include "Util.h"
#include "base/Logger.h"


#include <arpa/inet.h>
#include <iostream>
const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;
Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
  assert(epollFd_ > 0);
}
Epoll::~Epoll() {}
// 注册新描述符
void Epoll::epoll_add(const SP_Channel &request, int timeout) {
  int fd = request->get_fd();  // 获取文件描述符
  if (timeout > 0) {           // 如果超时时间大于0，添加定时器
    add_timer(request, timeout);
    fd2http_[fd] = request->getHolder();  // 将文件描述符和对应的HttpData绑定
  }
  struct epoll_event event;
  event.data.fd = fd;                   // 设置事件的文件描述符
  event.events = request->getEvents();  // 设置事件类型
  request->EqualAndUpdateLastEvents();  // 更新事件类型
  fd2chan_[fd] = request;  // 将文件描述符和对应的Channel绑定
  if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) <
      0) {                      // 将事件添加到epoll监听队列中
    perror("epoll_add error");  // 输出错误信息
    fd2chan_[fd].reset();       // 重置文件描述符和对应的Channel
  }
}
void Epoll::epoll_mod(const SP_Channel &request, int timeout) {
  if (timeout > 0) add_timer(request, timeout);
  int fd = request->get_fd();
  if (!request->EqualAndUpdateLastEvents()) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
      perror("epoll_mod error");
      fd2chan_[fd].reset();
    }
  }
}
void Epoll::add_timer(SP_Channel request_data, int timeout) {
  std::shared_ptr<HttpData> t = request_data->getHolder();
  if (t)
    timerManager_.addTimer(t, timeout);
  else
    LOG << "timer add fail";
}


void Epoll::epoll_del(const SP_Channel &request) {  //删除绑定的事件
  int fd = request->get_fd();
  struct epoll_event event;  // 用于表示一个epoll事件。
  event.data.fd = fd;
  event.events = request->getLastEvents();
  if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll_del error");
  }
  fd2chan_[fd].reset();
  fd2http_[fd].reset();
}
// 返回活跃事件数
std::vector<SP_Channel> Epoll::poll() {
  while (true) {
    int event_count =
        epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
    if (event_count < 0) perror("epoll wait error");
    std::vector<SP_Channel> req_data = getEventsRequest(event_count);
    if (req_data.size() > 0) return req_data;
  }
}
void Epoll::handleExpired() { timerManager_.handleExpiredEvent(); }
// 分发处理函数
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num) {
  std::vector<SP_Channel> req_data;  // 存储事件产生的Channel对象
  for (int i = 0; i < events_num; ++i) {
    // 获取有事件产生的描述符
    int fd = events_[i].data.fd;
    SP_Channel cur_req = fd2chan_[fd];  // 获取对应的Channel对象
    if (cur_req) {
      cur_req->set_revents(events_[i].events);  // 设置Channel对象的事件类型
      cur_req->set_events(0);       // 重置Channel对象的事件类型
      req_data.push_back(cur_req);  // 将Channel对象加入到req_data中
    } else {
      LOG << "SP cur_req is invalid";
    }
  }
  return req_data;  // 返回有事件产生的Channel对象
}


//
// Created by chao on 23-5-25.
//

#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H


#include <unordered_map>
#include <vector>
#include "Channel.h"

class Epoll {

    ~Epoll();

    void epoll_add(const SP_Channel &request);

    void epoll_mod(const SP_Channel &request);

    void epoll_del(const SP_Channel &request);

    void poll(std::vector<SP_Channel> &req);

public:
    Epoll();

private:
    int epollFd_; //就是⽤ epoll_create ⽅法返回的 epoll 句柄，这个是常识。
//    std::vector<epoll_event> events_;//存放 epoll_wait() 返回的活动事件（是⼀个结构体）
    std::unordered_map<int, SP_Channel> channelMap_;//负责记录⽂件描述符fd -> Channel 的映射，也帮忙保管所有注册在你这个 Poller 上的 Channel
};


#endif //WEBSERVER_EPOLL_H

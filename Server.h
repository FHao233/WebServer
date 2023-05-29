#ifndef SERVER_H
#define SERVER_H

#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
class Server {
 public:
  Server(EventLoop *loop, int threadNum, int port); // 构造函数，需要传入一个EventLoop对象、线程数和端口号
  ~Server() {}// 析构函数
  EventLoop *getLoop() const { return loop_; } // 返回EventLoop对象
  void start();// 启动服务器
  void handNewConn();// 处理新连接
  void handThisConn() { loop_->PollerMod(acceptChannel_); }// 处理当前连接
 private:
  EventLoop *loop_;// EventLoop对象
  int threadNum_;// 线程数
  std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;  // EventLoop线程池
  bool started_; // 服务器是否已经启动
  std::shared_ptr<Channel> acceptChannel_; // 接受连接的Channel对象
  int port_; // 端口号
  int listenFd_;// 监听套接字
  static const int MAXFDS = 100000; // 最大文件符数
};
#endif
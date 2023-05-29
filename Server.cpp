
#include "Server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <functional>

#include "Util.h"
#include "base/Logger.h"
Server::Server(EventLoop *loop, int threadNum, int port)
    : loop_(loop),            // 初始化EventLoop指针
      threadNum_(threadNum),  // 初始化线程数
      eventLoopThreadPool_(
          new EventLoopThreadPool(loop_, threadNum)),  // 初始化EventLoop线程池
      started_(false),                     // 初始化服务器状态
      acceptChannel_(new Channel(loop_)),  // 初始化接受连接的Channel对象
      port_(port),                         // 初始化端口号
      listenFd_(socket_bind_listen(port_))  // 创建监听套接字并绑定端口
{
  acceptChannel_->set_fd(listenFd_); // 设置接受连接的Channel对象的文件描述符
  handle_for_sigpipe();
  if (setSocketNonBlocking(listenFd_) < 0) {
    perror("set socket non block failed");
    abort();
  }
}

void Server::start() {
  eventLoopThreadPool_->start();// 启动EventLoop线程池
  acceptChannel_->set_events(EPOLLIN | EPOLLET);// 设置接受连接的Channel对象的事件类型
  acceptChannel_->set_read_handler(std::bind(&Server::handNewConn, this));// 设置接受连接的Channel对象的读事件回调函数
  acceptChannel_->set_conn_handler(std::bind(&Server::handThisConn, this));// 设置接受连接的Channel对象的连接事件回调函数
  loop_->PollerAdd(acceptChannel_, 0);// 将接受连接的Channel对象添加到EventLoop的Poller中
  started_ = true;// 设置服务器状态为已启动
}
void Server::handNewConn() {
  struct sockaddr_in client_addr;// 客户端地址结构体
  memset(&client_addr, 0, sizeof(struct sockaddr_in));// 初始化客户端地址结构体
  socklen_t client_addr_len = sizeof(client_addr); // 客户端地址结构体长度
  int accept_fd = 0;// 接受连接的套接字
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) { // 循环接受连接
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();// 获取下一个EventLoop对象
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"// 输出新连接的客户端地址和端口号
        << ntohs(client_addr.sin_port);
    if (accept_fd >= MAXFDS) {// 如果接受连接的套接字数目超过最大值
      close(accept_fd);// 关闭套接字
      continue; // 继续循环
    }
    // 设为非阻塞模式
    if (setSocketNonBlocking(accept_fd) < 0) {// 将受连接的套接字设置为非阻塞模式
      LOG << "Set non block failed!";
      return;
    }
    setSocketNodelay(accept_fd);// 设置TCP_NODELAY选项
    std::shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));// 创建HttpData对象
    req_info->getChannel()->set_holder(req_info);// 设置Channel对象的持有者
    loop->QueueInLoop(
        std::bind(&HttpData::newEvent,
                  req_info));  // 将新连接的处理任务添加到EventLoop的任务队列中
  }
  acceptChannel_->set_events(EPOLLIN | EPOLLET);// 重新设置接受连接的Channel对象的事件类型
}

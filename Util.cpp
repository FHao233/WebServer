#include "Util.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
const int MAX_BUFF = 4096;

ssize_t readn(int fd, void *buff, size_t n) {
  size_t nleft = n;          // 剩余需要读取的字节数
  ssize_t nread = 0;         // 已经读取的字节数
  ssize_t readSum = 0;       // 累计已经读取的字节数
  char *ptr = (char *)buff;  // 缓冲区指针
  while (nleft > 0) {        // 只要还有字节需要读取就继续循环
    if ((nread = read(fd, ptr, nleft)) < 0) {  // 读取数据
      if (errno == EINTR)  // 如果读取被中断，则继续读取
        nread = 0;
      else if (errno ==
               EAGAIN) {  // 如果当前没有数据可读，则返回已经读取的字节数
        return readSum;
      } else {  // 如果读取出错，则返回-1
        return -1;
      }
    } else if (nread == 0)  // 如果已经读取到文件末尾，则退出循环
      break;
    readSum += nread;  // 累加已经读取的字节数
    nleft -= nread;    // 更新剩余需要读取的字节数
    ptr += nread;      // 更新缓冲区指针
  }
  return readSum;  // 返回已经读取的字节数
}

ssize_t readn(int fd, std::string &inBuffer, bool &zero) {
  ssize_t nread = 0;
  ssize_t readSum = 0;
  while (true) {
    char buff[MAX_BUFF];
    if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
      if (errno == EINTR)
        continue;
      else if (errno == EAGAIN) {
        return readSum;
      } else {
        perror("read error");
        return -1;
      }
    } else if (nread == 0) {
      // printf("redsum = %d\n", readSum);
      zero = true;
      break;
    }
    // printf("before inBuffer.size() = %d\n", inBuffer.size());
    // printf("nread = %d\n", nread);
    readSum += nread;
    // buff += nread;
    inBuffer += std::string(buff, buff + nread);
    // printf("after inBuffer.size() = %d\n", inBuffer.size());
  }
  return readSum;
}
ssize_t readn(int fd, std::string &inBuffer) {
  ssize_t nread = 0;
  ssize_t readSum = 0;
  while (true) {
    char buff[MAX_BUFF];
    if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
      if (errno == EINTR)
        continue;
      else if (errno == EAGAIN) {
        return readSum;
      } else {
        perror("read error");
        return -1;
      }
    } else if (nread == 0) {
      // printf("redsum = %d\n", readSum);
      break;
    }
    // printf("before inBuffer.size() = %d\n", inBuffer.size());
    // printf("nread = %d\n", nread);
    readSum += nread;
    // buff += nread;
    inBuffer += std::string(buff, buff + nread);
    // printf("after inBuffer.size() = %d\n", inBuffer.size());
  }
  return readSum;
}
ssize_t writen(int fd, void *buff, size_t n) {//将缓冲区buff中的n个字节数据写入文件描述符fd中，返回值为写入的字节数。
  size_t nleft = n;
  ssize_t nwritten = 0;
  ssize_t writeSum = 0;
  char *ptr = (char *)buff;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
          continue;
        } else if (errno == EAGAIN) {
          return writeSum;
        } else
          return -1;
      }
    }
    writeSum += nwritten;
    nleft -= nwritten;
    ptr += nwritten;
  }
  return writeSum;
}

ssize_t writen(int fd, std::string &sbuff) {
  size_t nleft = sbuff.size();
  ssize_t nwritten = 0;
  ssize_t writeSum = 0;
  const char *ptr = sbuff.c_str();
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
          continue;
        } else if (errno == EAGAIN)
          break;
        else
          return -1;
      }
    }
    writeSum += nwritten;
    nleft -= nwritten;
    ptr += nwritten;
  }
  if (writeSum == static_cast<int>(sbuff.size()))
    sbuff.clear();
  else
    sbuff = sbuff.substr(writeSum);
  return writeSum;
}
// 这段代码的作用是在处理网络编程时，忽略SIGPIPE信号。在网络编程中，当一个进程向一个已经关闭写端的socket连接写数据时，
// 会触发SIGPIPE信号，如果不处理该信号，程序会异常退出。
void handle_for_sigpipe() {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));  // 初始化sigaction结构体
  sa.sa_handler = SIG_IGN;        // 忽略SIGPIPE信
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL)) return;  // 注册信号处理函数
}
// 将指定的文件描述符设置为非阻塞模式
int setSocketNonBlocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);  // 获取文件描述符的标志
  if (flag == -1) return -1;
  flag |= O_NONBLOCK;  // 设置文件描述符为非阻塞模式
  if (fcntl(fd, F_SETFL, flag) == -1) return -1;  // 设置文件描述符标志
  return 0;
}
// 这段代码的作用是设置TCP_NODELAY选项，用于禁用Nagle算法。
void setSocketNodelay(int fd) {  // 设置TCP_NODELAY选项
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}
// 这段代码的作用是设置SO_LINGER选项，用于控制socket关闭时的行为。当SO_LINGER选项被设置时，
// socket关闭时会等待段时间，以确保所有数据都被发送或接收完毕。如果在等待时间内数据没有被发送或接收完毕，那么socket会制关闭，丢弃未发送或未接收的数据。
void setSocketNoLinger(int fd) {
  struct linger
      linger_;  // SO_LINGER是一个socket选项，用于控制socket关闭时的行为。
  linger_.l_onoff = 1;    // l_onoff表示是否启用SO_LINGER选项
  linger_.l_linger = 30;  // l_linger表示等待时间，单位为秒。
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&linger_,
             sizeof(linger_));
}
void shutDownWR(int fd) { shutdown(fd, SHUT_WR); }
// 创建一个TCP服务器，并将其绑定到指定的端口上，开始监听客户端的连接请求
int socket_bind_listen(int port) {
  // 检查port值，取正确区间范围
  if (port < 0 || port > 65535) return -1;

  // 创建socket(IPv4 + TCP)，返回监听描述符
  int listen_fd = 0;
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

  // 消除bind时"Address already in use"错误
  int optval = 1;
  // SO_REUSEADDR是一个套接字选项，用于设置套接字的地址重用。当一个套接字关闭后，它会在一段时间内保持在TIME_WAIT状态，
  // 以确保所有的数据包都被传输完毕。在这段时间，如果有新的连接请求到达，那么新的连接将无法建立，因为该端口已经被占用。
  // 启用SO_REUSEADDR项后，可以在套接字关闭后立即重用该端口，而不必等待TIME_WAIT状态结束。这样可以避免端口资源的浪费，提高服务器的并发性能。
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                 sizeof(optval)) == -1) {
    close(listen_fd);
    return -1;
  }

  // 设置服务器IP和Port，和监听描述副绑定
  // 将一个IPv4地址和端口号绑定到一个套接字上。具体来说，它完成以下几个步骤：
  struct sockaddr_in
      server_addr;  // 创建一个sockaddr_in结构体对象server_addr，用于存储服务器的地址信息。
  bzero(
      (char *)&server_addr,
      sizeof(
          server_addr));  //将server_addr对象的内存清零，以确保其中的所有成员变量都被初始化为0。
  server_addr.sin_family = AF_INET;  // AF_INET，表示使用IPv4协议。
  server_addr.sin_addr.s_addr = htonl(
      INADDR_ANY);  // htonl(INADDR_ANY)，表示将服务器的IP地址设置为本地任意可用的IP地址。
  server_addr.sin_port = htons(
      (unsigned short)port);  // 表示将服务器的端口号设置为port参数指定的值。
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1)  //调用bind函数将listen_fd套接字与server_addr对象绑定，
  {
    close(listen_fd);
    return -1;
  }

  // 开始监听，最大等待队列长为LISTENQ
  // 作用是将listen_fd套接字设置为监听状态，以便接受客户端的连接请求。
  // 在服务器端编程中，listen函数通常是在bind函数之后调用的。
  // 它告诉操作系统该套接字已经准备好接受客户端连接请求，并指定了最大连接数。一旦listen函数调用成功，服务器就可以开始接受客户的连接请求了。
  if (listen(listen_fd, 2048) ==
      -1) {  // 调用listen函数将listen_fd套接字设置为监听状态，同时指定最大数为2048。这意味着该套接字可以同时处理2048个客户端连接请求。
    close(listen_fd);
    return -1;
  }

  // 无效监听描述符
  if (listen_fd == -1) {
    close(listen_fd);
    return -1;
  }
  return listen_fd;
}
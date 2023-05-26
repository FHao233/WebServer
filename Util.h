#include <cstdlib>
#include <string>
#pragma once
ssize_t readn(int fd, void *buff, size_t n); //从文件描述符fd中读取n个字节的数据到缓冲区buff中，返回值为读取的字节数。
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
void setSocketNoLinger(int fd);
void shutDownWR(int fd);
int socket_bind_listen(int port);
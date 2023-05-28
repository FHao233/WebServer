#include <cstdlib>
#include <string>
#pragma once
ssize_t readn(int fd, void *buff, size_t n); //从文件描述符fd中读取n个字节的数据到缓冲区buff中，返回值为读取的字节数。
ssize_t readn(
    int fd, std::string &inBuffer,
    bool &
        zero);  //从文件描述符fd中读取数据到字符串inBuffer中，直到读取到文件末尾或者读取出错。如果读取到文件末尾，则将zero设置为true，否则设置为false。返回值为读取的字节数。
ssize_t readn(int fd, std::string &inBuffer);//从文件描述符fd中读取数据到字符串inBuffer中，直到读取到文件末尾或者读取出错。返回值为读取的字节数。
ssize_t writen(int fd, void *buff, size_t n);//将缓冲区buff中的n个字节数据写入文件描述符fd中，返回值为写入的字节数。
ssize_t writen(int fd, std::string &sbuff);//将字符串sbuff中的数据写入文件描述符fd中，返回值为写入的字节数。
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
void setSocketNoLinger(int fd);
void shutDownWR(int fd);
int socket_bind_listen(int port);
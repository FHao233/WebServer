//
// Created by chao on 23-5-25.
//

#ifndef WEBSERVER_HTTPDATA_H
#define WEBSERVER_HTTPDATA_H
#include <sys/epoll.h>
#include <unistd.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "Timer.h"
class EventLoop;
class TimerNode;
class Channel;
// ProcessState：表示HTTP请求处理的不同阶段，如解析URI、解析HTTP头、接收HTTP请求、分析HTTP请求和完成HTTP请求等。
enum ProcessState {
  STATE_PARSE_URI = 1,  // 解析URI状态
  STATE_PARSE_HEADERS,  // 解析HTTP头状态
  STATE_RECV_BODY,      // 接收HTTP请求状态
  STATE_ANALYSIS,       // 分析HTTP请求状态
  STATE_FINISH          // 完成HTTP请求状态
};
// URIState：表示解URI的不同状态，如重新解析URI、解析URI错误和解析URI成功等。
enum URIState {
  PARSE_URI_AGAIN = 1,  // 重新解析URI状态
  PARSE_URI_ERROR,      // 解析URI错误状态
  PARSE_URI_SUCCESS,    // 解析URI成功状态
};
// HeaderState：表示解析HTTP头的不同状态，解析HTTP头成功、重新解析HTTP头和解析HTTP头错误等。
enum HeaderState {
  PARSE_HEADER_SUCCESS = 1,  // 解析HTTP头成功状态
  PARSE_HEADER_AGAIN,        // 重新解析HTTP头状态
  PARSE_HEADER_ERROR         // 解析HTTP头错误状态
};
// AnalysisState：表示分析HTTP请求的不同状态，如分析HTTP请求成功和分析HTTP请求错误等。
enum AnalysisState {
  ANALYSIS_SUCCESS = 1,  // 分析HTTP请求成功状态
  ANALYSIS_ERROR         // 分析HTTP请求错误状态
};
// ParseState表示HTTP头解析的不同状态，如HTTP头解析开始、HTTP头解析键、HTTP头解析冒号、HTTP头解析冒号后空格、
// HTTP头解析值、HTTP头解析回车、HTTP头解析换行、HTTP头解析结束回车和HTTP头解析结束换行等。
enum ParseState {
  H_START = 0,           // HTTP头解析开始状态
  H_KEY,                 // HTTP头解析键状态
  H_COLON,               //头解析冒号状态
  H_SPACES_AFTER_COLON,  // HTTP头解析冒号后空格状态
  H_VALUE,               // HTTP头解析值状态
  H_CR,                  // HTTP头解析回车状态
  H_LF,                  // HTTP头解析换行状态
  H_END_CR,              // HTTP头解析结束回车状态
  H_END_LF               // HTTP头解析结束换行状态
};
// ConnectionState：表示HTTP连接的不同状态，如已连接、正在断开连接和已断开连接等。
enum ConnectionState {
  H_CONNECTED = 0,  // 已连接状态
  H_DISCONNECTING,  // 正在断开连接状态
  H_DISCONNECTED    // 已断开连接状态
};
// HttpMethod：表示HTTP请求的不同方法，如POST请求、GET请求和HEAD请求等。
enum HttpMethod {
  METHOD_POST = 1,  // POST请求方法
  METHOD_GET,       // GET请求方法
  METHOD_HEAD       // HEAD请求方法
};
// HttpVersion：表示HTTP协议的不同版本，如HTTP/1.0版本和HTTP/1.1版本等。
enum HttpVersion {
  HTTP_10 = 1,  // HTTP/1.0版本
  HTTP_11       // HTTP/1.版本
};

// 定义了一个MimeType类，用于获取文件后缀对应的MIME类型。
class MimeType {
 private:
  static void init();  // 初始化函数 用于初始化mime哈希表。
  static std::unordered_map<std::string, std::string>
      mime;    // 存储文件后缀和对应MIME类型的哈希表
  MimeType();  // 构造函数，用于创建MimeType对象。
  MimeType(const MimeType &m);  // 拷贝构造函数 用于创建MimeType对象的副本。

 public:
  static std::string getMime(
      const std::string &suffix);  // 获取指定文件后缀对应的MIME类型。

 private:
  static pthread_once_t
      once_control;  // 线程控制变量，用于保证init()函数只被调用一次
};
class HttpData
    : public std::enable_shared_from_this<
          HttpData> {  // 解决对象自身需要获取自己的shared_ptr的问题。
 public:
  HttpData(EventLoop *loop,
           int connfd);  // 构造函数，创建HttpData对象  ~HttpData() {
                         // close(fd_); }  // 析构函数，关闭连接
  void reset();          // 重置HttpData对象
  void seperateTimer();  // 分离定时器
  void linkTimer(std::shared_ptr<TimerNode> mtimer) {  // 关联定时器
    // shared_ptr重载了bool, 但weak_ptr没有
    timer_ = mtimer;
  }
  std::shared_ptr<Channel> getChannel() { return channel_; }  // 获取Channel对象
  EventLoop *getLoop() { return loop_; }  // 获取EventLoop对象
  void handleClose();                     // 处理连接关闭事件
  void newEvent();                        // 处理新事件

 private:
  EventLoop *loop_;                             // EventLoop对象指针
  std::shared_ptr<Channel> channel_;            // Channel对象指针
  int fd_;                                      // 连接文件描述符
  std::string inBuffer_;                        // 输入缓冲区
  std::string outBuffer_;                       // 输出缓冲区
  bool error_;                                  // 是否出错
  ConnectionState connectionState_;             // 连接状态
  HttpMethod method_;                           // HTTP请求方法
  HttpVersion HTTPVersion_;                     // HTTP版本
  std::string fileName_;                        // 文件名
  std::string path_;                            // 文件路径
  int nowReadPos_;                              // 当前读取位置
  ProcessState state_;                          // 处理状态
  ParseState hState_;                           // 解析状态
  bool keepAlive_;                              // 是否保持连接
  std::map<std::string, std::string> headers_;  // HTTP头部信息
  std::weak_ptr<TimerNode> timer_;              // 定时器指针
  void handleRead();                            // 处理读事件
  void handleWrite();                           // 处理写事件
  void handleConn();                            // 处理连接事件
  void handleError(int fd, int err_num, std::string short_msg);  // 处理错误事件
  URIState parseURI();                                           // 解析URI
  HeaderState parseHeaders();       // 解析HTTP头部信息
  AnalysisState analysisRequest();  // 分析HTTP请求
};

#endif  // WEBSERVER_HTTPDATA_H

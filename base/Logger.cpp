#include "Logger.h"

#include <sys/time.h>

#include "AsyncLogging.h"
// 定义一个静态变量，用于保证 AsyncLogging 对象只被创建一次
static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
// 定义一个 AsyncLogging 对象指针
static AsyncLogging* AsyncLogger_;
// 定义日志文件名
std::string Logger::logFileName_ = "./WebServer.log";
// 用于保证 AsyncLogging 对象只被创建一次的函数
void once_init() {
  AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
  AsyncLogger_->start();
}

// 输出日志信息的函数
void output(const char* msg, int len) {
  // 保证 AsyncLogging 对象只被创建一次
  pthread_once(&once_control_, once_init);
  // 将日志信息追加到 AsyncLogging 对象中
  AsyncLogger_->append(msg, len);
}
// Logger 类的构造函数
Logger::Logger(const char* fileName, int line) : impl_(fileName, line) {}
// Logger 类的析构函数
Logger::~Logger() {
  // 将文件名和行号添加到日志信息中
  impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
  // 获取日志信息缓冲区
  const LogStream::Buffer& buf(stream().buffer());
  // 将日志信息追加到 AsyncLogging 对象中
  output(buf.data(), buf.length());
}
// Logger::Impl 类的构造函数
Logger::Impl::Impl(const char* fileName, int line)
    : stream_(), line_(line), basename_(fileName) {
  // 格式化时间
  formatTime();
}
// 格式化时间的函数
void Logger::Impl::formatTime() {
  struct timeval tv;
  time_t time;
  char str_t[26] = {0};
  // 获取当前时间
  gettimeofday(&tv, NULL);
  time = tv.tv_sec;
  // 将时间转换为本地时间
  struct tm* p_time = localtime(&time);
  //格式化时间字符串
  strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
  // 将时间字符串添加到日志信息中
  stream_ << str_t;
}

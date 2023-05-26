#ifndef LOGGER_H
#define LOGGER_H
#include <string>

#include "LogStream.h"
class AsyncLogging;
class Logger {
 public:
  Logger(const char *fileName, int line);
  ~Logger();
  LogStream &stream() { return impl_.stream_; }
  static void setLogFileName(std::string fileName) { logFileName_ = fileName; }
  static std::string getLogFileName() { return logFileName_; }

 private:
  // Logger的内部实现类Impl：负责把⽇志头信息写⼊logstream
  // 包含：时间戳，LogStream数据流，源⽂件⾏号，源⽂件名字
  // TODO:加⼊⽇志级别
  class Impl {
   public:
    Impl(const char *fileName, int line);
    void formatTime();

    LogStream stream_;
    int line_;
    std::string basename_;
  };
  Impl impl_;
  static std::string logFileName_;
};
// C/C++提供了三个宏来定位程序运⾏时的错误
// __FUNCTION__:返回当前所在的函数名
// __FILE__:返回当前的⽂件名
// __LINE__:当前执⾏⾏所在⾏的⾏号
#define LOG Logger(__FILE__, __LINE__).stream()
#endif
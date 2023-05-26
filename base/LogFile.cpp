#include "LogFile.h"

// LogFile类的构造函数，接受一个文件名basename和一个flushEveryN参数
LogFile::LogFile(const std::string& basename, int flushEveryN)
    : basename_(basename),        // 初始化basename_
      flushEveryN_(flushEveryN),  // 初始化flushEveryN_
      count_(0),                  // 初始化计数器count_
      mutex_(new MutexLock) {     // 初始化互斥锁mutex_
  file_.reset(
      new AppendFile(basename));  // 创建一个AppendFile对象，并将其赋值给file_
}

// LogFile类的析构函数
LogFile::~LogFile() {}

// 向日志文件中添加一行日志
void LogFile::append(const char* logline, int len) {
  MutexLockGuard lock(
      *mutex_);  // 创建一个MutexLockGuard对象，用于自动加锁和解锁
  append_unlocked(logline,
                  len);  // 调用append_unlocked函数，向日志文件中添加一行日志
}

// 刷新日志文件
void LogFile::flush() {
  MutexLockGuard lock(
      *mutex_);  // 创建一个MutexLockGuard对象，用于自动加锁和解锁
  file_->flush();  // 调用AppendFile类的flush函数，刷新日志文件
}

// 向日志文件中添加一行日志，不加锁
void LogFile::append_unlocked(const char* logline, int len) {
  file_->append(logline,
                len);  // 调用AppendFile类的append函数，向日志文件中添加一行日志
  ++count_;                      // 计数器加1
  if (count_ >= flushEveryN_) {  // 如果计数器达到了flushEveryN_，则刷新日志文件
    count_ = 0;                  // 重置数器
    file_->flush();  // 调用AppendFile类的flush函数，刷新日志文件
  }
}

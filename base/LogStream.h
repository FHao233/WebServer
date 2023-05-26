#ifndef LOGSTREAM_H
#define LOGSTREAM_H
#include <string.h>

#include <string>

#include "noncopyable.h"
class AsyncLogging;

// 定义两个缓冲区的大小
const int kSmallBuffer = 4000;         // 4K大小
const int kLargeBuffer = 4000 * 1000;  // 4M大小
template <int SIZE>
/**
 * 定义缓冲池，实现简单，开辟一个传入大小的字符数组，作为缓冲池
 */
class FixedBuffer : noncopyable {
 public:
  FixedBuffer() : cur_(data_) {}  // 构造函数，将cur_指针指向data_数组的起始位置
  ~FixedBuffer() {}
  // 向缓冲区中添加数据
  void append(const char* buf, size_t len) {
    if (avail() > static_cast<int>(
                      len)) {  // 如果缓冲区剩余空间足够，就将数据拷贝到缓冲区中
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }
  // 返回缓冲区中的数据
  const char* data() const { return data_; }
  // 返回缓冲区中的数据长度
  int length() const { return static_cast<int>(cur_ - data_); }
  // 返回当前指针位置
  char* current() { return cur_; }
  // 返回缓冲区中剩余的空间
  int avail() const { return static_cast<int>(end() - cur_); }
  // 将当前指针位置向后移动len个位置
  void add(size_t len) { cur_ += len; }
  // 重缓冲区，将当前指针位置指向缓冲区的起始位置
  void reset() { cur_ = data_; }
  // 将缓冲区清零
  void bzero() { memset(data_, 0, sizeof data_); }

 private:
  // 返回缓冲区的末尾位置
  const char* end() const { return data_ + sizeof data_; }
  char data_[SIZE];  // 缓冲区
  char* cur_;        // 当前指针位置
};
/**
 * logStream 类实际上只持有⼀个缓冲区
Buffer，由于⽇志输⼊的类型繁多，为了后端写⼊⽅便（也为了缓冲
区的格式统⼀），需要将输⼊的数据转换为 char 字符类型再进⾏写⼊。
*/
class LogStream : noncopyable {
 public:
  typedef FixedBuffer<kSmallBuffer> Buffer;  // 定义一个固定大小的缓冲区类型
  LogStream& operator<<(bool v) {  // 重载bool类型的输出运算符
    buffer_.append(v ? "1" : "0", 1);  // 将bool类型转换为字符串后添加到缓冲区中
    return *this;
  }
  LogStream& operator<<(short);           // 重载short类型的输出运算符
  LogStream& operator<<(unsigned short);  // 重载unsigned short类型的输出运算符
  LogStream& operator<<(int);
  LogStream& operator<<(unsigned int);
  LogStream& operator<<(long);
  LogStream& operator<<(unsigned long);
  LogStream& operator<<(long long);
  LogStream& operator<<(unsigned long long);

  LogStream& operator<<(const void*);
  LogStream& operator<<(double);
  LogStream& operator<<(long double);
  LogStream& operator<<(float v) {  // 重载float类型的输出运算符
    *this << static_cast<double>(v);  // 将float类型转换为double类型后输出
    return *this;
  }
  LogStream& operator<<(char v) {  // 重载char类型的输出运算符
    buffer_.append(&v, 1);         // 将char类型添加到缓冲区中
    return *this;
  }
  LogStream& operator<<(const char* str) {  // 重载const char*类型的运算符
    if (str)
      buffer_.append(str, strlen(str));  // 将字符串添加到缓冲区中
    else
      buffer_.append("(null)",
                     6);  // 如果字符串为空指针，则添加"(null)"到缓冲区中
    return *this;
  }
  LogStream& operator<<(
      const unsigned char* str) {  // 重载const unsigned char*类型的输出运算符
    return operator<<(reinterpret_cast<const char*>(
        str));  // 将const unsigned char*类型转换为const char*类型后输出
  }
  LogStream& operator<<(const std::string& v) {
    buffer_.append(
        v.c_str(),
        v.size());  // 将std::string类型转换为const char*类型后添加到缓冲区中
    return *this;
  }
  void append(const char* data, int len) {
    buffer_.append(data, len);
  }  // 向缓冲区中添加数据
  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

 private:
  void staticCheck();  // 静态检查函数

  template <typename T>
  void formatInteger(T);  // 格式化整数类型

  Buffer buffer_;

  static const int kMaxNumericSize = 32;  // 最大数字长度
};
#endif
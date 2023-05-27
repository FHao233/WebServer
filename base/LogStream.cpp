#include "LogStream.h"
#include <algorithm>
const char digits[] =
    "9876543210123456789";  // 数字字符数组，用于将数字转换为字符串
const char* zero = digits + 9;  // 数字字符数组中0的位置

// From muduo
template <typename T>
size_t convert(char buf[], T value) {  // 将整数类型转换为字符串类型
  T i = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 10);  // 取出最低位数字
    i /= 10;                             // 去掉最低位数字
    *p++ = zero[lsd];                    // 将数字字符添加到字符串中
  } while (i != 0);

  if (value < 0) {  //如果是负数，添加负号
    *p++ = '-';
  }
  *p = '\0';             // 字符串结尾添加'\0'
  std::reverse(buf, p);  // 反转字符串

  return p - buf;  // 返回字符串长度
}
template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;
template <typename T>
void LogStream::formatInteger(T v) {
  // buffer容不下kMaxNumericSize个字符的话会被直接丢弃
  if (buffer_.avail() >= kMaxNumericSize) {
    size_t len = convert(buffer_.current(), v);
    buffer_.add(len);
  }
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}
LogStream& LogStream::operator<<(int v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  formatInteger(v);
  return *this;
}
LogStream& LogStream::operator<<(double v) {  // 重载double类型的输出运算符
  if (buffer_.avail() >= kMaxNumericSize) {  // 如果缓冲区有足够的空间
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g",
                       v);  // 将double类型转换为字符串类型
    buffer_.add(len);       // 将字符串添加到缓冲区中  }
  }
  return *this;
}

LogStream& LogStream::operator<<(long double v) {
  if (buffer_.avail() >= kMaxNumericSize) {
    int len = snprintf(
        buffer_.current(), kMaxNumericSize, "%.12Lg",
        v);  //这句话是将long
             // double类型的变量v转换为字符串，并将结果存储在buffer_缓冲区中的当前位置，
    // 该字符串最多包含kNumericSize个字符，并且保留小数点后12位
    buffer_.add(len);
  }
  return *this;
}
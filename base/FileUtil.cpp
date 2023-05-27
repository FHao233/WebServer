#include "FileUtil.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
AppendFile::AppendFile(std::string filename): fp_(fopen(filename.c_str(), "ae")){
  // 构造函数,打开指定的 filename 文件,以追加模式打开。
  // 同时设置文件流的缓冲区,以提高写入效率。
  setbuffer(fp_, buffer_, sizeof(buffer_)); // 这行代码是为文件流 fp_ 设置缓冲区,缓冲区为 buffer_,大小为 sizeof buffer_ 字节。
}
AppendFile::~AppendFile() {
  // 析构函数,关闭文件流,释放资源。
  fclose(fp_);
}
void AppendFile::append(const char* logline, const size_t len) {
  // 向文件追加内容。
  // 首先调用 write() 函数尝试写入内容,写入成功后更新已写入的字节数。
  // 如果未写入完整,则继续调用 write() 函数,直到全部内容写入完成。
  // 在写入过程中,如果出现错误,则输出错误信息。
  size_t n = this->write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = this->write(logline + n, remain);
    if (x == 0) {
      int err = ferror(fp_);
      if (err) fprintf(stderr, "AppendFile::append() failed !\n");
      break;
    }
    n += x;
    remain = len - n;
  }
}
void AppendFile::flush() {
  // 刷新文件流缓冲区,将缓冲区内容写入文件。
  fflush(fp_);
}
size_t AppendFile::write(const char* logline, size_t len) {
  // 写入内容到文件。
  // 使用 fwrite_unlocked() 函数写入,返回实际写入的字节数。
  return fwrite_unlocked(logline, 1, len, fp_);
}
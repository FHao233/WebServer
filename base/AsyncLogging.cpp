#include "AsyncLogging.h"

#include <assert.h>

#include "LogFile.h"

AsyncLogging::AsyncLogging(const std::string logFileName_, int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(logFileName_),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_(),
      latch_(1) {
  assert(logFileName_.size() > 1);
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len) {
  MutexLockGuard lock(mutex_);  // 使用互斥锁保证线程安全
  if (currentBuffer_->avail() >
      len) {  // 如果当前缓区的可用空间足够存储待添加的日志信息
    currentBuffer_->append(logline, len);  // 直接将日志添加到当前缓冲区中
  } else {  // 如果当前缓冲区的可用空间不足
    buffers_.push_back(currentBuffer_);  // 将当前缓冲区添加到缓冲区列表中
    currentBuffer_.reset();              // 重置当前缓冲区
    if (nextBuffer_) {  // 如果下一个缓冲区(nextBuffer_)不为空
      currentBuffer_ =
          std::move(nextBuffer_);  // 将下一个缓冲区作为新的当前缓冲区
    } else {  // 如果下一个缓冲区(nextBuffer_)为空
      currentBuffer_.reset(
          new Buffer);  // 创建一个新的缓冲区作为当前缓冲区,如果前端写入速度太快了，一下子把两块缓冲都用完了，那么只好分配一块新的buffer,作当前缓冲，这是极少发生的情况
    }
    currentBuffer_->append(logline,
                           len);  // 将待添加的日志信息添加到当前缓冲区中
    cond_.notify();  // 通过条件变量通知线程有新的日志信息需要处理
  }
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);  // 断言线程正在运行
  latch_.countDown();  // 计数器减一，通知主线程子线程已经启动
  LogFile output(basename_);  // LogFile⽤于将⽇志写⼊⽂件，创建一个日志文件对象
  BufferPtr newBuffer1(new Buffer);  // 创建一个新的缓冲区对象
  BufferPtr newBuffer2(new Buffer);  // 创建另一个新的缓冲区对象
  newBuffer1->bzero();               // 将新缓冲区1清空
  newBuffer2->bzero();               // 将新缓冲区2清空
  BufferVector
      buffersToWrite;  //该vector属于后端线程，⽤于和前端的buffers进⾏交换
  buffersToWrite.reserve(16);  // 预留16个缓冲区的空间
  while (running_) {           // 循环直到线程停
    assert(newBuffer1 &&
           newBuffer1->length() == 0);  // 断言新缓冲区1不为空且长度为0
    assert(newBuffer2 &&
           newBuffer2->length() == 0);  // 断言新缓冲区2不为空且长度为0
    assert(buffersToWrite.empty());  // 断言后端缓冲区向量为空

    //将前端buffers_中的数据交换到buffersToWrite中
    {
      MutexLockGuard lock(mutex_);  // 加锁
      //每隔3s，或者currentBuffer满了，就将currentBuffer放⼊buffers_中
      if (buffers_.empty())  // unusual usage! 如果缓冲区向量为空
      {
        cond_.waitForSeconds(flushInterval_);  // 等待一段时间
      }
      buffers_.push_back(
          std::move(currentBuffer_));  // 将当前缓冲区对象加入缓冲区向量
      //   currentBuffer_.reset();
      currentBuffer_ =
          std::move(newBuffer1);  // 将新缓冲区1移动到当前缓冲区对象
      buffersToWrite.swap(buffers_);  // 交换缓冲区向量和待写入缓冲区向量
      if (!nextBuffer_) {
        nextBuffer_ = std::move(
            newBuffer2);  // 如果下一个缓冲区对象为空，则将新缓冲区2移动到下一个缓冲区对象
      }
    }
    assert(!buffersToWrite.empty());  // 断言待写入缓冲区向量不为空
    //如果队列中buffer数⽬⼤于25，就删除多余数据
    //避免⽇志堆积：前端⽇志记录过快，后端来不及写⼊⽂件
    if (buffersToWrite.size() > 25) {
      // TODO: 删除数据时加错误提示
      //只留原始的两个buffer，其余的删除
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }
    for (const auto& buffer : buffersToWrite) {  // 遍历待写入缓冲区向量
      output.append(
          buffer->data(),
          buffer->length());  // 将待写入缓冲区向量中的缓冲区写入日志文件
    }
    //重新调整buffersToWrite的⼤⼩，仅保留两个原始buffer
    if (buffersToWrite.size() > 2) {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }
    if (!newBuffer1) {                  // 如果新缓冲区1为空
      assert(!buffersToWrite.empty());  // 断言待写入缓冲区向量不为空
      newBuffer1 =
          buffersToWrite
              .back();  // 将待写入缓冲区向量中的最后一个缓冲区移动到新缓冲区1
      buffersToWrite.pop_back();  // 删除待写入缓冲区向量中的最后一个缓冲区
      newBuffer1->reset();  // 重置新缓冲区1
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = buffersToWrite.back();
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();  // 清空待写入缓冲区向量
    output.flush();          // 刷新日志文件
  }
  output.flush();  // 刷新日志文件
}

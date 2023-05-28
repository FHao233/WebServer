//
// Created by chao on 23-5-25.
//

#include "HttpData.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <iostream>

#include "Channel.h"
#include "EventLoop.h"
#include "Util.h"
#include "time.h"
using std::string;
pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;
//这样，当fd文件描述符可读时，epoll_wait函数会返回该事件，并且该事件只会被触发一次，之后需要重新添加到epoll监听队列中。
const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;              // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // ms
char favicon[555] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
    '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
    '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
    '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
    '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
    '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
    'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
    'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
    'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
    '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
    '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
    '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
    'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
    '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
    '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
    '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
    '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
    '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
    '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
    '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
    '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
    '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
    '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
    '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
    'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
    '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
    '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
    '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
    '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
    '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
    '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
    '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
    'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};

void MimeType::init() {
  mime[".html"] = "text/html";
  mime[".avi"] = "video/x-msvideo";
  mime[".bmp"] = "image/bmp";
  mime[".c"] = "text/plain";
  mime[".doc"] = "application/msword";
  mime[".gif"] = "image/gif";
  mime[".gz"] = "application/x-gzip";
  mime[".htm"] = "text/html";
  mime[".ico"] = "image/x-icon";
  mime[".jpg"] = "image/jpeg";
  mime[".png"] = "image/png";
  mime[".txt"] = "text/plain";
  mime[".mp3"] = "audio/mp3";
  mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string &suffix) {
  pthread_once(&once_control, MimeType::init);
  if (mime.find(suffix) == mime.end()) {
    return mime["default"];
  } else
    return mime[suffix];
}

// HttpData类的构造函数，接受一个EventLoop指针和一个连接文件描述符作为参数
HttpData::HttpData(EventLoop *loop, int connfd)
    : loop_(loop),
      channel_(new Channel(
          loop,
          connfd)),  // 创建一个Channel对象，将其指针赋值给channel_成员变量
      fd_(connfd),    // 将连接文件描述符赋值给fd_成员变量
      error_(false),  // 初始化error_成员变量为false
      connectionState_(
          H_CONNECTED),     // 初始化connectionState_员变量为H_CONNECTED
      method_(METHOD_GET),  // 初始化method_成员变量为METHOD_GET
      HTTPVersion_(HTTP_11),    // 初始化HTTPVersion_成员变量为HTTP_11
      nowReadPos_(0),           // nowReadPos_成员变量为0
      state_(STATE_PARSE_URI),  // 初始化state_成员变量为STATE_PARSE_URI
      hState_(H_START),         // 初始化hState_成员变量为H_START
      keepAlive_(false) {       // 初始化keepAlive_成员变量为false
  // 将handleRead()、handleWrite()和handleConn()函数绑定到Channel对象上
  channel_->set_read_handler(bind(&HttpData::handleRead, this));
  channel_->set_write_handler(bind(&HttpData::handleWrite, this));
  channel_->set_conn_handler(bind(&HttpData::handleConn, this));
}
// 重置HttpData对象的成员变量
void HttpData::reset() {
  fileName_.clear();         // 清空fileName_成员变量
  path_.clear();             // 清空path_成员变量
  nowReadPos_ = 0;           // 将nowReadPos_成员变量重置为0
  state_ = STATE_PARSE_URI;  // 将state_成员变量重置为STATE_PARSE_URI
  hState_ = H_START;         // 将hState_成员变量重置为H_START
  headers_.clear();          // 清空headers_成员变量
  if (timer_.lock()) {       // 如果timer_指针不为空
    std::shared_ptr<TimerNode> my_timer(
        timer_.lock());  // 将timer_指针转换为shared_ptr指针
    my_timer->clearReq();  // 调用定时器中的clearReq()函数清除请求信息
    timer_.reset();        // 将timer_指针重置为空
  }
}
void HttpData::seperateTimer() {
  // cout << "seperateTimer" << endl;
  if (timer_.lock()) {  // 如果timer_指针不为空
    std::shared_ptr<TimerNode> my_timer(
        timer_.lock());    // 将weak_ptr 转换为share_ptr
    my_timer->clearReq();  // 清除定时器关联的请求
    timer_.reset();        // 将timer_指针重置为空
  }
}
/**
 * 这段代码实现了 HttpData 类的读事件处理函数 handleRead()，用于处理 HTTP
 请求的读取和解析。 首先获取 channel_ 对应的事件类型 events_，然后进入一个
 do-while 循环，环中进行以下操作： 定义一个布尔变量
 zero，用于标记是否读到了文件结尾； 调用 readn()读取数据到 inBuffer_
 中，并将读取的数据长度保存到 read_num 中； 如果当前连接状态为
 H_DISCONNECTING，则清空 inBuffer_ 并跳出循环； 如果读取数据出错，则设置 error_
 为 true，调用 handleError() 处理错误，并跳出循环；
    如果读取到了文件结尾，则将连接状态设置为
 H_DISCONNECTING，如果读取的数据长度为 0，则跳出循环； 如果当前状态为
 STATE_PARSE_URI，则调用 parseURI() 函数解析 URI，根据返回值进行相应的处理；
    如果当前状态为 STATE_PARSE_HEADERS，则调用 parseHeaders()
 函数解析请求头，根据返回值进行相应的处理； 如果当前方法为 POST，则将状态设置为
 STATE_RECV_BODY； 如果当前状态为 STATE_RECV_BODY，则根据请求头中的
 Content-length 字段判断是否已经接收完整个请求体，如果没有则跳出循环；
    如果当前状态为 STATE_ANALYSIS，则调用 analysisRequest()
 函数分析请求，根据返回值进行相应的处理； 如果以上操作都没有出错且当前状态为
 STATE_FINISH，则重置 HttpData 对象并继续处理 inBuffer_ 中剩余的数据；
    如果以上操作都没有出错且连接状态不为 H_DISCONNECTED，则将 events_ 的 EPOLLIN
 位置为 1。 最后，如果没有出错且 outBuffer_ 中有数据，则调用 handleWrite()
 函数处理写事件。
*/
// 处理读事件
void HttpData::handleRead() {
  __uint32_t &events_ = channel_->getEvents();  // 获取 channel 对应的事件类型
  do {
    bool zero = false;  // 标记是否读到了文件结尾
    int read_num = readn(fd_, inBuffer_, zero);  // 读取数据到 inBuffer_ 中
    LOG << "Request: " << inBuffer_;             // 记录请求日志
    if (connectionState_ ==
        H_DISCONNECTING) {  // 如果连接状态为 H_DISCONNECTING，则空 inBuffer_
                            // 并跳出循环
      inBuffer_.clear();
      break;
    }
    if (read_num < 0) {  // 如果读取数据出错，则设置 error_ 为 true，调用
                         // handleError() 处理错误，并跳出循环
      perror("1");
      error_ = true;
      handleError(fd_, 400, "Bad Request");
      break;
    } else if (zero) {  // 如果读到了文件结尾
      // 有请求出现但是读不到数据，可能是 Request
      // Aborted，或者来自网络的数据没有达到等原因
      // 最可能是对端已经关闭了，统一按照对端已经关闭处理
      // error_ = true;
      connectionState_ = H_DISCONNECTING;  // 将连接状态设置为 H_DISCONNECTING
      if (read_num == 0) {  // 如果读取的数据长度为 0，则跳出循环
        break;
      }
    }
    if (state_ ==
        STATE_PARSE_URI) {  // 如果当前状态为 STATE_PARSE，则用 parseURI()
                            // 函数解析 URI，根据返回值进行相应的处理
      URIState flag = this->parseURI();
      if (flag == PARSE_URI_AGAIN) {
        break;
      } else if (flag == PARSE_URI_ERROR) {
        perror("2");
        LOG << "FD = " << fd_ << "," << inBuffer_ << "******";
        inBuffer_.clear();
        error_ = true;
        handleError(fd_, 400, "Bad Request");
        break;
      } else {
        state_ = STATE_PARSE_HEADERS;
      }
    }
    if (state_ ==
        STATE_PARSE_HEADERS) {  // 如果当前状态为 STATE_PARSE_HEADERS，则调用
                                // parseHeaders()
                                // 函数解析请求头，根据返回值进行相应的处理
      HeaderState flag = this->parseHeaders();
      if (flag == PARSE_HEADER_AGAIN) {
        break;
      } else if (flag == PARSE_HEADER_ERROR) {
        perror("3");
        error_ = true;
        handleError(fd_, 400, "Bad Request");
        break;
      }
      if (method_ ==
          METHOD_POST) {  // 如果当前方法为 POST，则将状态设置为 STATE_RECV_BODY
        // POST 方法准备
        state_ = STATE_RECV_BODY;
      } else {
        state_ = STATE_ANALYSIS;
      }
    }
    if (state_ ==
        STATE_RECV_BODY) {  // 如果当前状态为 STATE_RECV_BODY，则根据请求头中的
                            // Content-length
                            // 字段判断是否已经接收完整个请求体，如果没有则跳出循环
      int content_length = -1;
      if (headers_.find("Content-length") != headers_.end()) {
        content_length = stoi(headers_["Content-length"]);
      } else {
        error_ = true;
        handleError(fd_, 400, "Bad Request: Lack of argument (Content-length)");
        break;
      }
      if (static_cast<int>(inBuffer_.size()) < content_length) break;
      state_ = STATE_ANALYSIS;
    }
    if (state_ == STATE_ANALYSIS) {  // 如果当前状态为 STATE_ANALYSIS，则调用
                                     // analysisRequest()
                                     // 函数分析请求，根据返回值进行相应的处理
      AnalysisState flag = this->analysisRequest();
      if (flag == ANALYSIS_SUCCESS) {
        state_ = STATE_FINISH;
        break;
      } else {
        error_ = true;
        break;
      }
    }
  } while (false);
  if (!error_) {  // 如果没有出错
    if (outBuffer_.size() >
        0) {  // 如果 outBuffer_ 中有数据，则调用 handleWrite() 函数处理写事件
      handleWrite();
    }
    if (!error_ &&
        state_ ==
            STATE_FINISH) {  // 如果没有出错且当前状态为 STATE_FINISH，则重置
                             // HttpData 对象并继续处理 inBuffer_ 中剩余的数据
      this->reset();
      if (inBuffer_.size() > 0) {
        if (connectionState_ != H_DISCONNECTING) handleRead();
      }
    } else if (!error_ && connectionState_ !=
                              H_DISCONNECTED)  // 如果没有出错且连接状态不为
                                               // H_DISCONNECTED，则将 events_
                                               // 的 EPOLLIN 位置为 1
      events_ |= EPOLLIN;
  }
}
void HttpData::handleWrite() {
  if (!error_ && connectionState_ != H_DISCONNECTED) {
    auto &events_ = channel_->getEvents();  // 获取 channel 对应的事件类型
    if (writen(fd_, outBuffer_) <
        0) {  // 将 outBuffer_ 中的数据写入 fd_ 中，如果出错则设置 error_ 为
              // true，将 events_ 置为 0，并记录错误信息
      perror("writen");
      events_ = 0;
      error_ = true;
    }
    if (outBuffer_.size() > 0)
      events_ |= EPOLLOUT;  // 如果 outBuffer_ 中还有数据，则将 events_ 的
                            // EPOLLOUT 位置为 1
  }
}

void HttpData::handleConn() {
  seperateTimer();                        // 分离定时器
  auto &events_ = channel_->getEvents();  // 获取 channel 对应的事件类型
  if (!error_ && connectionState_ ==
                     H_CONNECTED) {  // 如果没有出错且连接状态为 H_CONNECTED
    if (events_ != 0) {              // 如果 events_ 不为 0
      int timeout =
          DEFAULT_EXPIRED_TIME;  // 设置超时时间为 DEFAULT_EXPIRED_TIME
      if (keepAlive_) {
        // 如果 keepAlive_ 为true，则将超时时间设置为 DEFAULT_KEEP_ALIVE_TIME
        timeout = DEFAULT_KEEP_ALIVE_TIME;
      }
      // 如果 events_ 同时包含 EPOLLIN 和 EPOLLOUT，则将events_ 置为 0，并将
      // EPOLLOUT 位置为 1
      if ((events_ & EPOLLIN) && (events_ & EPOLLOUT)) {
        events_ = __uint32_t(0);
        events_ |= EPOLLOUT;
      }
      events_ |= EPOLLET;  // 将 events_ 的 EPOLLET 位置为 1
      // 调用 loop_ 的 PollerMod() 函数修改 channel_对应的事件类型和超时时间
      loop_->PollerMod(channel_, timeout);
    } else if (keepAlive_) {
      //第 0 位和第 31
      //位分别设置为1，表示该文件描述符可读，并使用边缘触发模式监听该描述符上的事件
      events_ |= (EPOLLIN | EPOLLET);
      int timeout = DEFAULT_KEEP_ALIVE_TIME;
      loop_->PollerMod(channel_, timeout);
    }
    //这句话的意思是，如果 error_ 为假且 connectionState_ 等于
    // H_DISCONNECTING，并且 events_ 中包含 EPOLLOUT
    // 事件，则条件成立其中，error_ 表示是否发生错误，connectionState_
    //表示连接状态，H_DISCONNECTING 表示正在关闭连接，events_
    //表示文件描述符上的事件类型。如果这些条件都满足，那么这个条件就成立，可以执行相应的操作。这个条件通常用于处理关闭连接时的事件，例如发送完所有数据后关闭连接。
    else if (!error_ && connectionState_ == H_DISCONNECTING &&
             (events_ & EPOLLOUT)) {
      events_ = (EPOLLOUT | EPOLLET);
    } else {
      loop_->RunInLoop(bind(&HttpData::handleClose, shared_from_this()));
    }
  }
}
/**
 * 这段代码是一个 HTTP 请求解析函数，用于解析 HTTP 请求中的 URI（Uniform
Resource Identifier）部分。具体的解析过程如下： 首先从输入缓冲区 inBuffer_
中查找请求行的结束位置，即 \r 字符的位置。如果没有找到，则返回
PARSE_URI_AGAIN，表示需要继续读取数据。
如果找到了请求行的结束位置，则将请求行保存到 request_line
变量中，并从输入缓冲区中删除请求行所占的空间。
根据请求行中的方法名（GET、POST、HEAD）来确定请求的方法类型，并保存到 method_
变量中。如果请求行中没有包含这些方法名，则返回 PARSE_URI_ERROR，表示解析出错。
从请求行中查找 URI 的起始位置，并根据 URI 的格式解析出文件名和 HTTP
版本号。如果解析出错，则返回 PARSE_URI_ERROR。 如果 URI 中包含查询参数（即 ?
后面的部分），则将其从名中删除。 解析完成后，返回
PARSE_URI_SUCCESS，表示解析成功。 需要注意的是，这段代码只解析了 URI
部分，还需要进一步解析 HTTP 请求的其他部分，例如请求头和请求体。
*/
URIState HttpData::parseURI() {
  std::string &str = inBuffer_;  // 获取输入缓冲区的引用
  std::string cop = str;         //制输入缓冲区的内容
  // 读到完整的请求行再开始解析请求
  size_t pos = str.find('\r', nowReadPos_);  // 查找请求行结束位置
  if (pos < 0) {  // 如果没有找到，则需要继续读取数据
    return PARSE_URI_AGAIN;
  }
  // 去掉请求行所占的空间，节省空间
  std::string request_line = str.substr(0, pos);  // 获取请求行
  if (str.size() > pos + 1) {  // 如果还有数据未被解析，则删除请求行所占的空间
    str = str.substr(pos + 1);
  } else
    str.clear();  // 否清空输入缓冲区
  // Method
  int posGet = request_line.find("GET");    // 查找 GET 方法名的位置
  int posPost = request_line.find("POST");  // 查找 POST 方法名的位置
  int posHead = request_line.find("HEAD");  // 查找 HEAD 方法名的位置
  if (posGet >= 0) {  // 如果找到了 GET 方法名，则设置请求方法为 GET
    pos = posGet;
    method_ = METHOD_GET;
  } else if (posPost >= 0) {  // 如果找到了 POST 方法名，则设置请求方法为 POST
    pos = posPost;
    method_ = METHOD_POST;
  } else if (posHead >= 0) {  // 如果找到了 HEAD 方法名，则设置请求方法为 HEAD
    pos = posHead;
    method_ = METHOD_HEAD;
  } else {  // 如果没有找到任何方法名，则解析出错
    return PARSE_URI_ERROR;
  }
  // filename
  pos = request_line.find("/", pos);  // 查找 URI 起始位置
  if (pos < 0) {  // 如果没有找到 URI，则默认为 index.html
    fileName_ = "index.html";
    HTTPVersion_ = HTTP_11;
    return PARSE_URI_SUCCESS;
  } else {
    size_t _pos = request_line.find(' ', pos);  // 查找 URI 结束位置
    if (_pos < 0) {  // 如果没有找到 URI 结束位置，则解析出错
      return PARSE_URI_ERROR;
    } else {
      if (_pos - pos > 1) {  // 如果 URI 长度大于 1，则解析出文件名
        fileName_ = request_line.substr(pos + 1, _pos - pos - 1);
        size_t __pos = fileName_.find('?');  // 查找查询参数的位置
        if (__pos >= 0) {  // 如果找到了查询参数，则将其从文件名中删除
          fileName_ = fileName_.substr(0, __pos);
        }
      } else  // 否则默认为 index.html
        fileName_ = "index.html";
    }
    pos = _pos;
  }
  //这部分代码用于解析HTTP请求中的HTTP版本号，并根据版本号设置HTTP版本。如果解析出错，则返回PARSE_URI_ERROR，否则返回PARSE_URI_SUCCESS。
  pos = request_line.find("/", pos);
  if (pos < 0)
    return PARSE_URI_ERROR;
  else {
    if (request_line.size() - pos <= 3)
      return PARSE_URI_ERROR;
    else {
      std::string ver = request_line.substr(pos + 1, 3);
      if (ver == "1.0") {
        HTTPVersion_ = HTTP_10;
      } else if (ver == "1.1")
        HTTPVersion_ = HTTP_11;
      else
        return PARSE_URI_ERROR;
    }
  }
  return PARSE_URI_SUCCESS;
}
HeaderState HttpData::parseHeaders() {
  string &str = inBuffer_;  // 获取输入缓冲区的引用
  int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
  int now_read_line_begin = 0;                // 当前读取行的起始位置
  bool notFinish = true;                      // 是否还需要继续解析
  size_t i = 0;                               // 当前读取位置
  for (; i < str.size() && notFinish; ++i) {  // 遍历输入缓冲区中的数据
    switch (hState_) {  // 根据当前状态进行不同的处理
      case H_START: {   // 开始解析
        if (str[i] == '\n' || str[i] == '\r') break;  // 如果是空行，则跳过
        hState_ = H_KEY;          // 否则开始解析键名
        key_start = i;            // 记录键名的起始位置
        now_read_line_begin = i;  // 记录当前行的起始位置
        break;
      }
      case H_KEY: {           // 解析键名
        if (str[i] == ':') {  // 如果遇到冒号，则表示键名解析完成
          key_end = i;        // 记录键名的结束位置
          if (key_end - key_start <= 0)
            return PARSE_HEADER_ERROR;  // 如果键名为空，则解析出错
          hState_ = H_COLON;            // 否则开始解析冒号
        } else if (str[i] == '\n' ||
                   str[i] == '\r')  // 如果遇到换行符，则解析出错
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_COLON: {         // 解析冒号
        if (str[i] == ' ') {  // 如果遇到空格，则表示冒号解析完成
          hState_ = H_SPACES_AFTER_COLON;  // 开始解析值
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_SPACES_AFTER_COLON: {  // 解析值
        hState_ = H_VALUE;          // 开始解析值
        value_start = i;            // 记录值的起始位置
        break;
      }
      case H_VALUE: {          // 解析值
        if (str[i] == '\r') {  // 如果遇到回车符，则表示值解析完成
          hState_ = H_CR;      // 开始解析回车符
          value_end = i;       // 记录值的结束位置
          if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
        } else if (i - value_start > 255)  // 如果值的长度超过了255，则解析出错
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_CR: {             // 解析回车符
        if (str[i] == '\n') {  // 如果遇到换行符，则表示一行解析完成
          hState_ = H_LF;  // 开始解析换行符
          string key(str.begin() + key_start,
                     str.begin() + key_end);  // 获取键名
          string value(str.begin() + value_start,
                       str.begin() + value_end);  // 获取值
          headers_[key] = value;    // 将键值对存入headers_中
          now_read_line_begin = i;  // 记录当前行的起始位置
        } else
          return PARSE_HEADER_ERROR;  // 否则解析出错
        break;
      }
      case H_LF: {             // 解析换行符
        if (str[i] == '\r') {  // 如果遇到回车符，则表示一行解析完成
          hState_ = H_END_CR;  //开始解析回车符
        } else {
          key_start = i;  // 否则开始解析下一行的键名
          hState_ = H_KEY;
        }
        break;
      }
      case H_END_CR: {  // 解析回车符
        if (str[i] == '\n') {  // 如果遇到换行符，则表示所有头部解析完成
          hState_ = H_END_LF;  // 开始解析换行符
        } else
          return PARSE_HEADER_ERROR;  // 否则解析出错
        break;
      }
      case H_END_LF: {            // 解析换行符
        notFinish = false;        // 所有头部解析完成
        key_start = i;            // 记录当前行的起始位置
        now_read_line_begin = i;  // 记录当前行的起始位置
        break;
      }
    }
  }
  if (hState_ == H_END_LF) {      // 如果所有头部解析完成
    str = str.substr(i);          // 删除已经解的数据
    return PARSE_HEADER_SUCCESS;  // 返回解析成功
  }
  str = str.substr(now_read_line_begin);  // 否则继续读取数据
  return PARSE_HEADER_AGAIN;              // 返回需要继续解析
}
AnalysisState HttpData::analysisRequest() {
  if (method_ == METHOD_POST) {  // 如果是POST请求
    // TODO: 处理POST请求
  } else if (method_ == METHOD_GET ||
             method_ == METHOD_HEAD) {  // 如果是GET或HEAD请求
    string header;                      // 定义头部字符串
    header += "HTTP/1.1 200 OK\r\n";    // 添加状态行
    // 如果请求头中包含Connection字段,并且值为Keep-Alive
    if (headers_.find("Connection") != headers_.end() &&
        (headers_["Connection"] == "Keep-Alive" ||
         headers_["Connection"] == "keep-alive")) {
      keepAlive_ = true;  // 标记为长连接
      // 添加Keep-Alive头部字段
      header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" +
                std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    }
    int dot_pos = fileName_.find('.');  // 查找文件名中的点号位置
    string filetype;
    if (dot_pos < 0) {                          //没有找到点号
      filetype = MimeType::getMime("default");  // 使用默认的MIME类型
    } else
      // 根据文件后缀获取MIME类型
      filetype = MimeType::getMime(fileName_.substr(dot_pos));
    // echo test
    if (fileName_ == "hello") {  // 如果请求的是/hello
                                 // 返回Hello World
      outBuffer_ =
          "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
      return ANALYSIS_SUCCESS;
    }
    // 如果请求的是/favicon.ico
    if (fileName_ == "favicon.ico") {
      // 添加Content-Type头部字段
      header += "Content-Type: image/png\r\n";
      // 添加Content-Length头部字段
      header += "Content-Length: " + std::to_string(sizeof favicon) + "\r\n";
      // 添加Server头部字段
      header += "Server: FHao's Web Server\r\n";
      header += "\r\n";  // 头部结束
      // 将头部添加到输出缓冲区
      outBuffer_ += header;
      // 将favicon添加到输出缓冲区
      outBuffer_ += string(favicon, favicon + sizeof favicon);
      return ANALYSIS_SUCCESS;
    }
    struct stat sbuf;  //存储文件的状态信息
                       // 获取文件信息
    if (stat(fileName_.c_str(), &sbuf) < 0) {
      header.clear();
      handleError(fd_, 404, "Not Found!");  // 处理404错误
      return ANALYSIS_ERROR;
    }
    header += "Content-Type: " + filetype + "\r\n";  // 添加Content-Type头部字段
    header += "Content-Length: " + std::to_string(sbuf.st_size) +
              "\r\n";  // 添加Content-Length头部字段
    header += "Server: FHao's Web Server\r\n";  // 添加Server头部字段
    // 头部结束
    header += "\r\n";
    outBuffer_ += header;  //头部添加到输出缓冲区
    if (method_ == METHOD_HEAD)
      return ANALYSIS_SUCCESS;  // 如果是HEAD请求，直接返回
    int src_fd = open(
        fileName_.c_str(), O_RDONLY,
        0);  // 打开文件调用C语言中的open函数来打开文件，并将文件描述符存储在src_fd变量中
    if (src_fd < 0) {  // 如果打开文件失败
      outBuffer_.clear();
      handleError(fd_, 404, "Not Found!");  // 处理404错误
      return ANALYSIS_ERROR;
    }
    // 将文件映射到内存中
    // 将以src_fd表示的文件的前sbuf.st_size个字节映射到内存中，并将映射后的内存地址存储在mmapRet变量中。
    // 映射后的内存区域只能被当前进程读取和修改，而对该内存区域的修改不会影响到原文件。
    void *mmapRet = mmap(
        NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd,
        0);  //使用C语言中的mmap函数将文件映射到内存中，并将映射后的内存地址存储在mmapRet变量中。
    close(src_fd);                    // 关闭文件
    if (mmapRet == (void *)-1) {      // 如果映射失败
      munmap(mmapRet, sbuf.st_size);  // 取消映射
      outBuffer_.clear();
      handleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    char *src_addr = static_cast<char *>(mmapRet);  // 获取映射后的地址
    outBuffer_ += string(
        src_addr, src_addr + sbuf.st_size);  // 将文件内容添加到输出缓冲区
    munmap(mmapRet, sbuf.st_size);           // 取消映射
    return ANALYSIS_SUCCESS;
  }
  return ANALYSIS_ERROR;
}
void HttpData::handleError(int fd, int err_num, string short_msg) {
  short_msg = " " + short_msg;
  char send_buff[4096];
  string body_buff, header_buff;
  body_buff += "<html><title>哎~出错了</title>";
  body_buff += "<body bgcolor=\"ffffff\">";
  body_buff += std::to_string(err_num) + short_msg;
  body_buff += "<hr><em> FHao's Web Server</em>\n</body></html>";
  header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_msg + "\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: Close\r\n";
  header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
  header_buff += "Server: FHao's Web Server\r\n";
  ;
  header_buff += "\r\n";
  // 错误处理不考虑writen不完的情况
  //使用sprintf函数将HTTP响应报文的头部信息存储在send_buff缓冲区中。
  sprintf(send_buff, "%s", header_buff.c_str());
  // 使用writen函数将send_buff缓冲区中的数据写入到fd文件描述符中。fd是一个已连接的套接字文件描述符，表示与客户端建立的TCP连接。
  // strlen函数用于获取send_buff缓冲区中的字符串长度，以作为writen函数的第三个参数，表示要写入的数据长度。
  writen(fd, send_buff, strlen(send_buff));
  sprintf(send_buff, "%s", body_buff.c_str());  //写入body
  writen(fd, send_buff, strlen(send_buff));
}
void HttpData::handleClose() {
  connectionState_ = H_DISCONNECTED;
  std::shared_ptr<HttpData> guard(shared_from_this());
  loop_->PollerDel(channel_);
}
void HttpData::newEvent() {
  channel_->set_events(DEFAULT_EVENT);
  loop_->PollerAdd(channel_, DEFAULT_EXPIRED_TIME);
}

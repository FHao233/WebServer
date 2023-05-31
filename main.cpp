#include <getopt.h>

#include <string>

#include "EventLoop.h"
#include "Server.h"
#include "base/Logger.h"

int main(int argc, char *argv[]) {
  int threadNum = 4;
  int port = 8081;

  std::string logPath = "./WebServer.log";
  // parse args
  int opt;
  const char *str = "t:l:p:";
  while (
      // getopt(argc, argv,
      // str)是一个C/C++函数，用于解析命令行参数。其中，argc表示命令行参数的个数，argv是指向命令行参数字符串数组的指针，
      // str是一个包含选项字符的字符串。该函数会依次解析命令行参数，并返回下一个选项字符。如果所有选项字符都已经解析完毕，
      //则返回-1。在解析令行参数时，可以使用冒号(:)来指定需要参数的选项字符。例如，"a:b:c"表示选项字符a需要一个参数，
      //选项字符b需要两个参数，选项字符c不需要参数。在解析命令行参数时，如果遇到需要参数的选项字符，但是提供参数，则会返回问号(?)。
      (opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        threadNum = atoi(optarg);
        break;
      }
      case 'l': {
        logPath = optarg;
        if (logPath.size() < 2 || optarg[0] != '/') {
          printf("logPath should start with \"/\"\n");
          abort();
        }
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  Logger::setLogFileName(logPath);
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !";
#endif
  EventLoop mainLoop;
  Server myHTTPServer(&mainLoop, threadNum, port);
  myHTTPServer.start();
  mainLoop.Loop();
  return 0;
}
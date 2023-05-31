#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
int main(int argc, const char** argv) {
  std::string fileName_ = "index.html";
  std::cout << fileName_ << std::endl;
   int src_fd = open(
    ("../resources/"+fileName_).c_str(), O_RDONLY,
      0);  //
//   int src_fd = open(fileName_.c_str(), O_RDONLY,
//                     0);  //
  std::cout << src_fd << std::endl;
}
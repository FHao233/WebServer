//
// Created by chao on 23-5-25.
//
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include "Thread.h"
#include "CurrentThread.h"
namespace CurrentThread {
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "default";
}
pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }
void Thread::setDefaultName() {

}

Thread::Thread(const Thread::ThreadFunc &, const std::string &name) {

}

Thread::~Thread() {

}

void Thread::start() {

}

int Thread::join() {
    return 0;
}

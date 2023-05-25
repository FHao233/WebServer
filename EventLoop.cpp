//
// Created by chao on 23-5-25.
//
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "EventLoop.h"
#include "Epoll.h"
auto EventLoop::CreateEventfd() -> int {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
//        LOG << "Failed in eventfd";
//        abort();
    }
    return evtfd;
}
EventLoop::EventLoop() :
        is_looping_(false),
        poller_(new Epoll()),
        event_fd_(CreateEventfd()),
        is_event_handling_(false),
        is_calling_pending_functions_(false),
        thread_id_(CurrentThread::tid())
        {

}

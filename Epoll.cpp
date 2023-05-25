//
// Created by chao on 23-5-25.
//

#include "Epoll.h"

Epoll::Epoll(const std::unordered_map<int, sp_Channel> &channelMap) : channelMap_(channelMap) {}

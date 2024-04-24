/*
 * @file        epoll_server.cpp
 * @brief       Brief description of the file
 * @author      nhmt
 * @date        2024/03/18
 */

#include "epoll_server.h"
EpollServer::EpollServer(int maxEvent) : epollFd(epoll_create(512)), events(maxEvent) {
    assert(epollFd >= 0 && events.size() > 0);
}
EpollServer::~EpollServer() {
    close(epollFd);
}

bool EpollServer::AddFd(int fd, uint32_t event) {
    if (fd < 0)return false;
    epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = event;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
}
bool EpollServer::ModFd(int fd, uint32_t event) {
    if (fd < 0)return false;
    epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = event;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}
bool EpollServer::DelFd(int fd) {
    if (fd < 0)return false;
    epoll_event ev = { 0 };
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev);
}
int EpollServer::Wait(int timeout) {
    return epoll_wait(epollFd, &events[0], static_cast<int>(events.size()), timeout);
}
int EpollServer::GetEventFd(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

uint32_t EpollServer::GetEvents(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].events;
}


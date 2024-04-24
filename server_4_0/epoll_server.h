/*
 * @file        epoll_server.h
 * @brief       Brief description of the class
 * @author      nhmt
 * @date        2024/03/18
 */

#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/epoll.h>
#include <cassert>
#include <iostream>
using std::cout;
using std::endl;

class EpollServer {
public:
    explicit EpollServer(int maxEvent = 1024);

    ~EpollServer();

    bool AddFd(int fd, uint32_t event);

    bool ModFd(int fd, uint32_t event);

    bool DelFd(int fd);

    int Wait(int timeout = -1);

    int GetEventFd(size_t i) const;

    uint32_t GetEvents(size_t i) const;

private:
    int epollFd;

    std::vector<struct epoll_event> events;
};


#endif /* EPOLL_SERVER_H */
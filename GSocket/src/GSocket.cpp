/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GSocket.h
 * Author: 10256
 * 
 * Created on 2017年3月13日, 上午11:28
 */

#include "GSocket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/select.h>
#include <stdexcept>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <netinet/tcp.h>
#include <errno.h>

using namespace gavinsocket;
using namespace std;
using namespace bytebuf;

extern int errno;

GSocket::GSocket(const int& port) : m_socketFd(-1) {
    memset(&m_servaddr, 0, sizeof (m_servaddr));
    m_servaddr.sin_port = htons(port);
    m_servaddr.sin_family = AF_INET;
}

GSocket::GSocket(const GSocket& orig) {
}

GSocket::~GSocket() {
    Close();
}

void GSocket::Socket() {
    if (m_socketFd < 0) {
        if ((m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            string errMsg = "GSocket::GSocket(): socket: ";
            throw runtime_error(errMsg + strerror(errno));
        }
    }
}

bool GSocket::isConnected(const int& fd) {
    if (fd < 0)
        return false;

    struct tcp_info info;
    int len = sizeof (info);
    getsockopt(fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) & len);
    return info.tcpi_state == TCP_ESTABLISHED;
}

void GSocket::Write(const int& fd, ByteBuffer* src, const size_t& size) {
    if (src->remaining() < size)
        throw runtime_error("GSocket::Write(): data space remaining is less than the size to write");

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_mutex);
#else
    std::unique_lock<std::mutex> lk(m_mutex);
#endif
    size_t actualNumSent = 0;
    uint8_t* _ptr = src->array();
    int currNumSent;
    for (; actualNumSent < size;) {
        if (!isConnected(fd)) {
            throw SocketException("GSocketClient::Write(): connection closed");
        }
        currNumSent = write(fd, _ptr + actualNumSent, size - actualNumSent);
        if (currNumSent < 0) {
            string errMsg = "GSocket::Write(): write: ";
            throw SocketException(errMsg + strerror(errno));
        }
        actualNumSent += currNumSent;
    }
    src->movePosition(actualNumSent);
}

void GSocket::Close() {
    if (m_socketFd > 0) {
#ifdef UseBoostMutex
        boost::unique_lock<boost::mutex> lk(m_mutex);
#else
        std::unique_lock<std::mutex> lk(m_mutex);
#endif
        close(m_socketFd);
        m_socketFd = -1;
    }
}
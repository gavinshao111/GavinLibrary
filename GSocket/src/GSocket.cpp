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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/select.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <netinet/tcp.h>
#include <cerrno>

#include "GSocket.h"

using namespace gsocket;
using namespace bytebuf;

extern int errno;
const static int ConnectionRefusedErrCode = 111;

GSocket::GSocket(const std::string& ip, const int& port) : m_socketFd(-1){
    memset(&m_servaddr, 0, sizeof (m_servaddr));
    m_servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    m_servaddr.sin_port = htons(port);
    m_servaddr.sin_family = AF_INET;
    Connect();
}

GSocket::GSocket(const int& fd, const struct sockaddr& clientaddr) : m_socketFd(fd) {
    memcpy(&m_clientaddr, &clientaddr, sizeof (struct sockaddr));
}

GSocket::GSocket(const GSocket& orig) {
    throw SocketException("GSocket's copy constructor not allowed to be called");
}

GSocket::~GSocket() {
    Close();
}

void GSocket::Connect(/*const size_t& timeout = 0*/) {
    if (isConnected())
        return;

    if ((m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::string errMsg = "GSocket::Connect(): socket: ";
        throw SocketException(errMsg.append(strerror(errno)));
    }
    if (connect(m_socketFd, (struct sockaddr*) &m_servaddr, sizeof (m_servaddr)) < 0) {
        if (ConnectionRefusedErrCode == errno)
            throw SocketConnectRefusedException();
        std::string errMsg = "GSocket::Connect(): connect: ";
        throw SocketException(errMsg.append(strerror(errno)));
    }
}

bool GSocket::isConnected() const {
    if (m_socketFd < 0)
        return false;

    struct tcp_info info;
    int len = sizeof (info);
    getsockopt(m_socketFd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) & len);
    return info.tcpi_state == TCP_ESTABLISHED;
}

void GSocket::Write(ByteBuffer& src, const size_t& size) {
    if (src.remaining() < size)
        throw SocketException("GSocket::Write(): data space remaining is less than the size to write");

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_mutex);
#else
    std::unique_lock<std::mutex> lk(m_mutex);
#endif
    size_t actualNumSent = 0;
    const uint8_t* writeBuf = src.array();
    int currNumSent;
    for (; actualNumSent < size;) {
        if (!isConnected()) {
            throw SocketException("GSocketClient::Write(): connection closed");
        }
        currNumSent = write(m_socketFd, writeBuf + actualNumSent, size - actualNumSent);
        if (currNumSent < 0) {
            std::string errMsg = "GSocket::Write(): write: ";
            throw SocketException(errMsg.append(strerror(errno)));
        }
        actualNumSent += currNumSent;
    }
    src.movePosition(actualNumSent);
}

void GSocket::Write(ByteBuffer& src) {
    Write(src, src.remaining());
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

void GSocket::Read(ByteBuffer& data, const size_t& size, const size_t& timeout/* = 0*/) {
    if (data.remaining() < size)
        throw SocketException("GSocket::Read(): data space remaining is less than the size to read");

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_mutex);
#else
    std::unique_lock<std::mutex> lk(m_mutex);
#endif
    uint8_t buf[size];
    size_t actualNumRead = 0;
    int currNumRead;
    struct timeval* timeLeft;

    if (0 == timeout)
        timeLeft = NULL;
    else {
        struct timeval tv = {timeout / 1000, 0};
        timeLeft = &tv;
    }
    for (; size > actualNumRead;) {

        FD_ZERO(&m_readFds);
        FD_SET(m_socketFd, &m_readFds);
        int err = select(m_socketFd + 1, &m_readFds, NULL, NULL, timeLeft);
        if (0 > err) {
            std::string errMsg = "GSocketClient::Read(): select: ";
            throw SocketException(errMsg.append(strerror(errno)));
        } else if (0 == err) {
            throw SocketTimeoutException();
        } else if (!FD_ISSET(m_socketFd, &m_readFds)) {
            throw SocketException("GSocketClient::Read(): FD_ISSET fail");
        }
        currNumRead = read(m_socketFd, buf, size - actualNumRead);
        if (0 > currNumRead) { // if u send data first but he didn't read and then u block to read, then he close connection, return value < 0
            std::string errMsg = "GSocketClient::Read(): connection may be closed. read: ";
            throw SocketException(errMsg.append(strerror(errno)));
        }
        if (0 == currNumRead) {
            throw SocketException("GSocketClient::Read(): connection closed");
        }
        actualNumRead += currNumRead;
        data.put(buf, 0, currNumRead);

        if (NULL != timeLeft && 0 == timeLeft->tv_sec && 0 == timeLeft->tv_usec)
            throw SocketTimeoutException();
    }
}


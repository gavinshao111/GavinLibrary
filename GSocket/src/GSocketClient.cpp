/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GSocketClient.cpp
 * Author: 10256
 * 
 * Created on 2017年3月13日, 上午11:23
 */

#include "GSocketClient.h"
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace gavinsocket;
using namespace bytebuf;

extern int errno;

const static int ConnectionRefusedErrCode = 111;

GSocketClient::GSocketClient(const string& ip, const int& port, const bool& isBlock/* = false*/) :
GSocket(port, isBlock) {
    if (0 == ip.compare("localhost"))
        m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_aton(ip.c_str(), (struct in_addr*) &m_servaddr.sin_addr);
}

//GSocketClient::GSocketClient(const GSocketClient& orig) {
//}

GSocketClient::~GSocketClient() {
}

bool GSocketClient::Connect(/*const size_t& timeout = 0*/) {
    if (isConnected())
        return true;
    
    Socket();
    if (connect(m_socketFd, (struct sockaddr*) &m_servaddr, sizeof (m_servaddr)) == -1) {
        if (ConnectionRefusedErrCode == errno)
            return false;
        string errMsg = "GSocketClient::Connect(): connect";
        cout << "errno: " << errno << endl;
        throw runtime_error(errMsg + strerror(errno));
        //        perror("GSocketClient::GSocketClient(): connect");
        //        throw runtime_error("GSocketClient::GSocketClient(): connect return -1");
    }
    return true;
//    if (!m_isBlock) {
//        int flags = fcntl(m_socketFd, F_GETFL, 0); //获取建立的sockfd的当前状态（非阻塞）
//        fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK); //将当前sockfd设置为非阻塞    
//    }
//
//    //    FD_ZERO(&m_readFds);
//    FD_ZERO(&m_writeFds);
//    //    FD_SET(m_socketFd, &m_readFds);
//    FD_SET(m_socketFd, &m_writeFds);
//    //    FD_SET(STDIN_FILENO, &m_readFds);
//    struct timeval tv = {timeout / 1000000, timeout % 1000000};
//    int err = select(m_socketFd + 1, NULL, &m_writeFds, NULL, &tv);
//    if (0 > err) {
//        //        perror("select");
//        //        throw runtime_error("select fail");
//        string errMsg = "GSocketClient::Connect(): select: ";
//        throw runtime_error(errMsg + strerror(errno));
//    } else if (0 == err) {
//        throw SocketException("GSocketClient::Connect(): time out");
//        //        cout << "select time out" << endl;
//    } else if (!FD_ISSET(m_socketFd, &m_writeFds)) {
//        throw SocketException("GSocketClient::Connect(): FD_ISSET fail");
//    }
}

bool GSocketClient::Read(ByteBuffer* data, const size_t& size, const size_t& timeout/* = 0*/) {
    if (data->remaining() < size)
        throw runtime_error("GSocketClient::Read(): data space remaining is less than the size to read");

    uint8_t buf[size];
    size_t actualNumRead = 0;
    int currNumRead;
    struct timeval* timeLeft;
    struct timeval tv = {timeout / 1000000, timeout % 1000000};

    if (0 == timeout)
        timeLeft = NULL;
    else
        timeLeft = &tv;

    for (; size > actualNumRead;) {

        FD_ZERO(&m_readFds);
        //    FD_ZERO(&m_writeFds);
        FD_SET(m_socketFd, &m_readFds);
        //    FD_SET(m_socketFd, &m_writeFds);
        //    FD_SET(STDIN_FILENO, &m_readFds);
        int err = select(m_socketFd + 1, &m_readFds, NULL, NULL, timeLeft);
        if (0 > err) {
            string errMsg = "GSocketClient::Read(): select: ";
            throw SocketException(errMsg + strerror(errno));
        } else if (0 == err) {
            //            cout << "[WARN] GSocketClient::Read(): select time out" << endl;
            return false;
        } else if (!FD_ISSET(m_socketFd, &m_readFds)) {
            throw SocketException("GSocketClient::Read(): FD_ISSET fail");
        }
        currNumRead = read(m_socketFd, buf, size - actualNumRead);
        if (0 > currNumRead) {  // if u send data first but he didn't read and then u block to read, then he close connection, return value < 0
            string errMsg = "GSocketClient::Read(): connection may be closed. read: ";
            throw SocketException(errMsg + strerror(errno));
        }
        if (0 == currNumRead) {
            throw SocketException("GSocketClient::Read(): connection closed");
        }
        actualNumRead += currNumRead;
        data->put(buf, 0, currNumRead);
        
        if (NULL != timeLeft && 0 == timeLeft->tv_sec && 0 == timeLeft->tv_usec)
            return false;
    }
    return true;
}

void GSocketClient::Write(ByteBuffer* src, const size_t& size) {
    GSocket::Write(m_socketFd, src, size);
}

void GSocketClient::Write(ByteBuffer* src) {
    Write(src, src->remaining());
}

bool GSocketClient::isConnected() {
    return GSocket::isConnected(m_socketFd);
}
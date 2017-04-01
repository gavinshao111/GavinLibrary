/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GSocketServer.cpp
 * Author: 10256
 * 
 * Created on 2017年3月13日, 下午1:48
 */

#include "GSocketServer.h"
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>

using namespace gavinsocket;
using namespace bytebuf;

GSocketServer::GSocketServer(const int& port)
: GSocket(port) {
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (0 != bind(m_socketFd, (struct sockaddr *)&m_servaddr, sizeof(m_servaddr))) {
        perror("bind");
        throw std::runtime_error("bind fail");
    }
    
    if (0 != listen(m_socketFd, 20)) {
        perror("listen");
        throw std::runtime_error("listen fail");
    }
    
    
}

//GSocketServer::GSocketServer(const GSocketServer& orig) {
//}

GSocketServer::~GSocketServer() {
}

void GSocketServer::Write(const int& fd, ByteBuffer* data, const size_t& size) {
    GSocket::Write(fd, data, size);
}
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

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <boost/smart_ptr/make_shared_object.hpp>

#include "GSocketServer.h"

using namespace gavinsocket;
using namespace bytebuf;

extern int errno;

GSocketServer::GSocketServer(const int& port) {

    if ((m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::string errMsg = "GSocketServer::GSocketServer(): socket: ";
        errMsg.append(strerror(errno));
        throw SocketException(errMsg);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof (servaddr));
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (0 != bind(m_socketFd, (struct sockaddr *) &servaddr, sizeof (servaddr))) {
        std::string errMsg = "GSocketServer::GSocketServer(): bind: ";
        errMsg.append(strerror(errno));
        throw SocketException(errMsg);
    }

    if (0 != listen(m_socketFd, 20)) {
        std::string errMsg = "GSocketServer::GSocketServer(): listen: ";
        errMsg.append(strerror(errno));
        throw SocketException(errMsg);
    }
}



//GSocketServer::GSocketServer(const GSocketServer& orig) {
//}

GSocketServer::~GSocketServer() {
    Close();
}

boost::shared_ptr<GSocket> GSocketServer::Accept() {
    struct sockaddr clientaddr;
    socklen_t addrlen = sizeof (clientaddr);
    int clientfd = accept(m_socketFd, &clientaddr, &addrlen);
    if (-1 == clientfd) {
        std::string errMsg = "GSocketServer::Accept(): accept: ";
        errMsg.append(strerror(errno));
        throw SocketException(errMsg);
    }
    /*
     * because GSocket(const int& fd, const struct sockaddr& clientaddr) is private, 
     * we can't use boost::make_shared to construct a GSocket;
     */
    return boost::shared_ptr<GSocket>(new GSocket(clientfd, clientaddr));
}

void GSocketServer::Close() {
    if (m_socketFd > 0) {
        close(m_socketFd);
        m_socketFd = -1;
    }
}

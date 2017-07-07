/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   socketserver.h
 * Author: 10256
 *
 * Created on 2017年3月13日, 下午1:48
 */

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <boost/shared_ptr.hpp>

#include "socket.h"
namespace gsocket {
    
class socketserver {
public:
    socketserver(const int& port);
    boost::shared_ptr<socket> accept();
    virtual ~socketserver();
    void close();
    
private:
    socketserver(const socketserver& orig);
    int m_socketFd;
};

}

#endif /* SOCKETSERVER_H */


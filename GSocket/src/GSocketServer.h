/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GSocketServer.h
 * Author: 10256
 *
 * Created on 2017年3月13日, 下午1:48
 */

#ifndef GSOCKETSERVER_H
#define GSOCKETSERVER_H

#include <boost/shared_ptr.hpp>

#include "GSocket.h"
namespace gsocket {
    
class GSocketServer {
public:
    GSocketServer(const int& port);
    boost::shared_ptr<GSocket> Accept();
    virtual ~GSocketServer();
    void Close();
    
private:
    GSocketServer(const GSocketServer& orig);
    int m_socketFd;
};

}

#endif /* GSOCKETSERVER_H */


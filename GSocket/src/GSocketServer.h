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

#include "GSocket.h"
namespace gavinsocket {
    
class GSocketServer : public GSocket {
public:
    GSocketServer(const int& port, const bool& isBlock = false);
//    GSocketServer(const GSocketServer& orig);
    virtual ~GSocketServer();
    void Write(const int& fd, bytebuf::ByteBuffer* data, const size_t& size);
private:
    

};

}

#endif /* GSOCKETSERVER_H */


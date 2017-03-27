/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GSocketClient.h
 * Author: 10256
 *
 * Created on 2017年3月13日, 上午11:23
 */

#ifndef GSOCKETCLIENT_H
#define GSOCKETCLIENT_H

#include "GSocket.h"
#include <string>
#include <sys/select.h>

namespace gavinsocket {

    class GSocketClient : public GSocket {
    public:
        GSocketClient(const std::string& ip, const int& port, const bool& isBlock = false);
        //        GSocketClient(const GSocketClient& orig);
        virtual ~GSocketClient();

        /**
         * check if connected already, if yes return true, else call socket() if necessary then call connect();
         * @return true if successful, false if refused. other cases throw exception
         */
        bool Connect(/*const size_t& timeout = 0*/);
        
        /**
         * @param timeout 0 means block to read.
         * @return true if successful, false if timeout
         */
        bool Read(bytebuf::ByteBuffer* src, const size_t& size, const size_t& timeout = 0);
        
        void Write(bytebuf::ByteBuffer* src, const size_t& size);
        
        /**
         * send the data at this src.position, 
         * if successful, the src.position is incremented by size.
         * 连接被对方关闭会抛出 SocketException
         */
        void Write(bytebuf::ByteBuffer* src);
        bool isConnected();
    private:
        fd_set m_writeFds;
    };
}

#endif /* GSOCKETCLIENT_H */


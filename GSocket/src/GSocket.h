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

#ifndef GSOCKET_H
#define GSOCKET_H

#include <string>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdexcept>
#include "ByteBuffer.h"

namespace gavinsocket {
    class SocketException : public std::runtime_error {
    public:

        SocketException(const SocketException& e) : std::runtime_error(e.what()) {
        }

        SocketException(const std::string& reason) : std::runtime_error(reason) {
        }

        SocketException() : std::runtime_error("") {
        }

        virtual ~SocketException() throw () {
        };
    };


    
    class GSocket {
    public:
        virtual ~GSocket();
        void Close();

    private:

    protected:
        GSocket(const int& port, const bool& isBlock = false);
        GSocket(const GSocket& orig);
        int m_socketFd;
        bool m_isBlock;
        struct sockaddr_in m_servaddr;
        fd_set m_readFds;
        
        /**
         * send the data at this src.position, 
         * if successful, the src.position is incremented by size.
         * @param fd
         * @param src
         * @param size
         */
        void Write(const int& fd, bytebuf::ByteBuffer* src, const size_t& size);
        void Socket();
        bool isConnected(const int& fd);
    };
}

#endif /* GSOCKET_H */



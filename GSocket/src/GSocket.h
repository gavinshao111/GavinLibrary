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
#include <stdexcept>
#ifdef UseBoostMutex
#include <boost/thread/mutex.hpp>
#else
#include <mutex>
#endif

#include "ByteBuffer.h"

namespace gsocket {

    class SocketException : public std::runtime_error {
    public:

        SocketException(const SocketException& e) : std::runtime_error(e.what()) {
        }

        SocketException(const std::string& message) : std::runtime_error(message) {
        }

        SocketException() : std::runtime_error("") {
        }

        virtual ~SocketException() throw () {
        };
    };

    class SocketConnectRefusedException : public SocketException {
    };

    class SocketTimeoutException : public SocketException {
    };

    class GSocket {
    public:
        virtual ~GSocket();
        void Close();
        void Connect(/*const size_t& timeout = 0*/);
        /**
         * connect to a specific server
         * @param ip
         * 
         * @param port
         */
        GSocket(const std::string& ip, const int& port);
//        GSocket(const GSocket& orig);
        /**
         * 
         * @param data
         * @param size
         * @param timeout millisecond, 0 means block to read, 
         * @throws SocketTimeoutException if timeout
         */
        void Read(bytebuf::ByteBuffer& data, const size_t& size, const size_t& timeout = 0);

        void Write(bytebuf::ByteBuffer& src, const size_t& size);

        /**
         * send the data at this src.position, 
         * if successful, the src.position is incremented by size.
         * 连接被对方关闭会抛出 SocketException
         * @param src read only
         * @param size
         */
        void Write(bytebuf::ByteBuffer& src);
        bool isConnected() const;

    private:
        GSocket(const int& fd, const struct sockaddr& clientaddr);
        int m_socketFd;
        struct sockaddr m_clientaddr;

        struct sockaddr_in m_servaddr;
        fd_set m_readFds;

#ifdef UseBoostMutex
        boost::mutex m_mutex;
#else
        std::mutex m_mutex;
#endif
        friend class GSocketServer;
    };
}

#endif /* GSOCKET_H */



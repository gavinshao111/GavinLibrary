/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   socket.h
 * Author: 10256
 *
 * Created on 2017年3月13日, 上午11:28
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <netinet/in.h>
#include <stdexcept>
#include <memory>
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
    public:
        SocketConnectRefusedException(const SocketConnectRefusedException& e) : SocketException(e.what()) {
        }

        SocketConnectRefusedException() : SocketException("connect refused") {
        }

        virtual ~SocketConnectRefusedException() throw () {
        };
    };

    class SocketTimeoutException : public SocketException {
    public:
        SocketTimeoutException(const SocketTimeoutException& e) : SocketException(e.what()) {
        }

        SocketTimeoutException() : SocketException("socket timeout") {
        }

        virtual ~SocketTimeoutException() throw () {
        };
    };

    class socket {
    public:
        /**
         * connect to a specific server
         * @param ip
         * 
         * @param port
         */
        socket(const std::string& ip, const int& port);
        
        virtual ~socket();
        
        void close();
        
        void connect(/*const size_t& timeout = 0*/);
        /**
         * 
         * @param dest
         * @param size
         * @param timeout millisecond, 0 means block to read, 
         * @throws SocketTimeoutException if timeout
         * timeout单位为ms，此函数会个问题，在超时前，即使socket被外部关闭，内部的select仍阻塞。
         */
        void read_old(bytebuf::ByteBuffer& dest, const size_t& size, const size_t& timeout = 0);

        /**
         * 
         * @param dest
         * @param size
         * @param timeout second, 0 means block to read, 
         * @throws SocketTimeoutException if timeout
         * 
         * 解决了read_old无限阻塞的问题。
         * timeout单位为s，内部循环调用超时为0.1s的select函数，循环中每次都检查连接是否正常，否则抛异常。
         * 循环直到时间超过timeout。这样可以避免即使连接断开，函数仍阻塞的情况。
         */
        void read(bytebuf::ByteBuffer& dest, const size_t& size, const size_t& timeout = 0);
        
        /**
         * 将tcp缓冲区数据读完
         * @return 
         */
        std::shared_ptr<bytebuf::ByteBuffer> read_left();

        void write(bytebuf::ByteBuffer& src, const size_t& size);

        /**
         * send the data at this src.position, 
         * if successful, the src.position is incremented by size.
         * 连接被对方关闭会抛出 SocketException
         * @param src read only
         * @param size
         */
        void write(bytebuf::ByteBuffer& src);
        bool isConnected() const;

    private:
        socket(const int& fd, const struct ::sockaddr& clientaddr);
        socket(const socket& orig);
                
        int m_socketFd;
        struct ::sockaddr m_clientaddr;

        struct ::sockaddr_in m_servaddr;
        fd_set m_readFds;

#ifdef UseBoostMutex
        boost::mutex m_connectMutex;
        boost::mutex m_readMutex;
        boost::mutex m_writeMutex;
#else
        std::mutex m_connectMutex;
        std::mutex m_readMutex;
        std::mutex m_writeMutex;
#endif
        friend class socketserver;
    };
}

#endif /* SOCKET_H */



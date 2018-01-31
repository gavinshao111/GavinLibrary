/*
 *  
 * read 中调用select实现超时返回机制，在select阻塞期间，另一个线程关闭socket，select不会有任何反应：
 * If a file descriptor being monitored by select() is closed in another thread, 
 * the result is unspecified. On some UNIX systems, select() unblocks and returns, 
 * with an indication that the file descriptor is ready (a subsequent I/O operation 
 * will likely fail with an error, unless another the file descriptor reopened between 
 * the time select() returned and the I/O operations was performed). 
 * On Linux (and some other systems), closing the file descriptor in another thread 
 * has no effect on select(). In summary, any application that relies on a particular 
 * behavior in this scenario must be considered buggy.
 * 
 */

/* 
 * File:   socket.h
 * Author: 10256
 * 
 * Created on 2017年3月13日, 上午11:28
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include "socket.h"

using namespace gsocket;
using namespace bytebuf;

const static char* class_name = "socket";
extern int errno;
const static int ConnectionRefusedErrCode = 111;
const static int BadFileDescriptor = 9;

socket::socket(const std::string& ip, const int& port) : m_socketFd(-1) {
    memset(&m_servaddr, 0, sizeof (m_servaddr));
    m_servaddr.sin_addr.s_addr = ::inet_addr(ip.c_str());
    m_servaddr.sin_port = ::htons(port);
    m_servaddr.sin_family = AF_INET;
    connect();
}

socket::socket(const int& fd, const struct ::sockaddr& clientaddr) : m_socketFd(fd) {
    ::memcpy(&m_clientaddr, &clientaddr, sizeof (struct ::sockaddr));
}

socket::~socket() {
    close();
}

void socket::connect(/*const size_t& timeout = 0*/) {
    if (isConnected())
        return;

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_connectMutex);
#else
    std::unique_lock<std::mutex> lk(m_connectMutex);
#endif
    if ((m_socketFd = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::string errMsg = "socket::connect(): socket: ";
        throw SocketException(errMsg.append(::strerror(errno)));
    }
    if (::connect(m_socketFd, (struct ::sockaddr*) & m_servaddr, sizeof (m_servaddr)) < 0) {
        if (ConnectionRefusedErrCode == errno)
            throw SocketConnectRefusedException();
        std::string errMsg = "socket::connect(): connect: ";
        throw SocketException(errMsg.append(::strerror(errno)));
    }
}

bool socket::isConnected() const {
    if (m_socketFd < 0)
        return false;

    struct ::tcp_info info;
    int len = sizeof (info);
    ::getsockopt(m_socketFd, ::IPPROTO_TCP, TCP_INFO, &info, (::socklen_t *) & len);
    return info.tcpi_state == ::TCP_ESTABLISHED;
}

void socket::write(ByteBuffer& src, const size_t& size) {
    if (src.remaining() < size)
        throw SocketException("socket::write(): data space remaining is less than the size to write");

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_writeMutex);
#else
    std::unique_lock<std::mutex> lk(m_writeMutex);
#endif
    size_t actualNumSent = 0;
    const uint8_t* writeBuf = src.array();
    int currNumSent;
    for (; actualNumSent < size;) {
        if (!isConnected()) {
            throw SocketException("socket::write(): connection closed");
        }
        currNumSent = ::write(m_socketFd, writeBuf + actualNumSent, size - actualNumSent);
        if (currNumSent < 0) {
            std::string errMsg = "socket::write(): write: ";
            throw SocketException(errMsg.append(::strerror(errno)));
        }
        actualNumSent += currNumSent;
    }
    src.movePosition(actualNumSent);
}

void socket::write(ByteBuffer& src) {
    write(src, src.remaining());
}

void socket::close() {
    if (m_socketFd > 0) {
        //#ifdef UseBoostMutex
        //        boost::unique_lock<boost::mutex> lk(m_connectMutex);
        //        boost::unique_lock<boost::mutex> lk2(m_readMutex);
        //        boost::unique_lock<boost::mutex> lk3(m_writeMutex);
        //#else
        //        std::unique_lock<std::mutex> lk(m_connectMutex);
        //        std::unique_lock<std::mutex> lk2(m_readMutex);
        //        std::unique_lock<std::mutex> lk3(m_writeMutex);
        //#endif
        ::close(m_socketFd);
        m_socketFd = -1;
    }
}

void socket::read_old(ByteBuffer& dest, const size_t& size, const size_t& timeout/* = 0*/) {
    if (dest.remaining() < size)
        throw SocketException("socket::read(): data space remaining is less than the size to read");

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_readMutex);
#else
    std::unique_lock<std::mutex> lk(m_readMutex);
#endif
    uint8_t buf[size];
    size_t actualNumread = 0;
    int currNumread;
    struct ::timeval* timeLeft;

    struct ::timeval tv = {(time_t) timeout / 1000, 0};
    if (0 == timeout)
        timeLeft = NULL;
    else {
        timeLeft = &tv;
    }
    for (; size > actualNumread;) {

        FD_ZERO(&m_readFds);
        FD_SET(m_socketFd, &m_readFds);
        int err = ::select(m_socketFd + 1, &m_readFds, NULL, NULL, timeLeft);
        if (0 > err) {
            std::string errMsg = "socket::read(): select: ";
            throw SocketException(errMsg.append(::strerror(errno)));
        } else if (0 == err) {
            throw SocketTimeoutException();
        } else if (!FD_ISSET(m_socketFd, &m_readFds)) {
            throw SocketException("socket::read(): FD_ISSET fail");
        }
        currNumread = ::read(m_socketFd, buf, size - actualNumread);
        if (0 > currNumread) { // if u send data first but he didn't read and then u block to read, then he close connection, return value < 0
            std::string errMsg = "socket::read(): connection may be closed. read: ";
            throw SocketException(errMsg.append(::strerror(errno)));
        }
        if (0 == currNumread) {
            throw SocketException("socket::read(): connection closed");
        }
        actualNumread += currNumread;
        dest.put(buf, 0, currNumread);

        if (NULL != timeLeft && 0 == timeLeft->tv_sec && 0 == timeLeft->tv_usec)
            throw SocketTimeoutException();
    }
}

void socket::read(ByteBuffer& dest, const size_t& size, const size_t& timeout/* = 0*/) {
    if (dest.remaining() < size)
        throw SocketException("socket::read(): data space remaining is less than the size to read");

#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_readMutex);
#else
    std::unique_lock<std::mutex> lk(m_readMutex);
#endif
    uint8_t buf[size];
    size_t actualNumread = 0;
    int currNumread;
    struct ::timeval t_100ms = {100 / 1000, 0};

    for (; size > actualNumread;) {
        for (size_t t_100msUsed = 0;; usleep(900)) {
            if (!this->isConnected())
                throw SocketException("socket::read(): connection closed");
            // select返回后会把以前加入的但并无事件发生的fd清空
            FD_ZERO(&m_readFds);
            FD_SET(m_socketFd, &m_readFds);
            int err = ::select(m_socketFd + 1, &m_readFds, NULL, NULL, &t_100ms);
            if (0 > err) {
                if (errno == BadFileDescriptor)
                    throw SocketException("socket::read(): connection closed");
                std::string errMsg = "socket::read(): select: ";
                throw SocketException(errMsg.append(::strerror(errno)));
            } else if (0 == err) {
                if (timeout == 0) {
                    continue;
                }
                t_100msUsed++;
                if (t_100msUsed >= timeout * 10)
                    throw SocketTimeoutException();
            } else if (!FD_ISSET(m_socketFd, &m_readFds)) {
                throw SocketException("socket::read(): FD_ISSET fail");
            } else
                break;
        }
        currNumread = ::read(m_socketFd, buf, size - actualNumread);
        if (0 > currNumread) { // if u send data first but he didn't read and then u block to read, then he close connection, return value < 0
            std::string errMsg = "socket::read(): connection may be closed. read: ";
            throw SocketException(errMsg.append(::strerror(errno)));
        }
        if (0 == currNumread) {
            throw SocketException("socket::read(): connection closed");
        }
        actualNumread += currNumread;
        dest.put(buf, 0, currNumread);
    }
}

std::shared_ptr<bytebuf::ByteBuffer> socket::read_left() {
    if (!isConnected()) {
        return nullptr;
    }
#ifdef UseBoostMutex
    boost::unique_lock<boost::mutex> lk(m_readMutex);
#else
    std::unique_lock<std::mutex> lk(m_readMutex);
#endif
    // todo: 缓冲区数据大于0x400，循环读取拼接ByteBuffer返回
    auto data = std::make_shared<bytebuf::ByteBuffer>(0x400);
    ::ssize_t read_count = ::read(m_socketFd, data->array(), data->remaining());
    if (0 > read_count) { // if u send data first but he didn't read and then u block to read, then he close connection, return value < 0
        throw SocketException(std::string(class_name) + "::" + __func__ + ": ::read fail: " + ::strerror(errno));
    }
    if (0 == read_count) {
        throw SocketException("socket::read(): connection closed");
    }
    data->limit(read_count);
    return data;
}
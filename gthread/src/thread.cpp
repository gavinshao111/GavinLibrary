
/* 
 * File:   thread.cpp
 * Author: 10256
 * 
 * Created on 2017年12月6日, 下午7:32
 */

#include "thread.h"
#include <boost/make_shared.hpp>
using namespace gavin;

thread::thread() {
}

thread::thread(const thread& orig) {
}

thread::~thread() {
    if (m_thread) {
        m_thread->interrupt();
        m_thread->join();
        m_thread.reset();
    }
}

void thread::start() {
    if (!m_thread)
        m_thread = boost::make_shared<boost::thread>(&thread::run, this);
}


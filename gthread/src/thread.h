
/* 
 * File:   thread.h
 * Author: 10256
 *
 * Created on 2017年12月6日, 下午7:32
 */

#ifndef GTHREAD_H
#define GTHREAD_H

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace gavin {

    class thread {
    public:
        thread();
        thread(const thread& orig);
        virtual ~thread();
        
        void start();
    protected:
        virtual void run() = 0;
        
    private:
        boost::shared_ptr<boost::thread> m_thread;
    };
}
#endif /* GTHREAD_H */


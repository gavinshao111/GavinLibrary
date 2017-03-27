/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SyncSubscriber.h
 * Author: 10256
 *
 * Created on 2017年3月24日, 下午1:27
 */

#ifndef SYNCSUBSCRIBER_H
#define SYNCSUBSCRIBER_H

#include "Subscriber.h"
namespace gmqtt {
class SyncSubscriber : public Subscriber {
public:
    SyncSubscriber(const std::string& serverURI,
                const std::string& clientId,
                const std::string& username,
                const std::string& password);
//    SyncSubscriber(const SyncSubscriber& orig);
    virtual ~SyncSubscriber();
        /**
         * 
         * @param out_data payload will be wrote into this argument, exception throwed if in_data doesn't have enough space.
         * @param timeout The length of time to wait for a message in seconds.
         * @return true if successful, false if timeout
         */
        bool receive(bytebuf::ByteBuffer& out_data, const size_t& timeout);
    
private:

};
}
#endif /* SYNCSUBSCRIBER_H */


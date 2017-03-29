/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AsyncSubscriber.h
 * Author: 10256
 *
 * Created on 2017年3月24日, 上午10:52
 */

#ifndef ASYNCSUBSCRIBER_H
#define ASYNCSUBSCRIBER_H

#include "Subscriber.h"

namespace gmqtt {
    typedef void (*msgarrvd_t)(const std::string& message); 

    class AsyncSubscriber : public Subscriber {
    public:
        AsyncSubscriber(const std::string& serverURI,
                const std::string& clientId,
                const std::string& username,
                const std::string& password);
//        AsyncSubscriber(const AsyncSubscriber& orig);
        virtual ~AsyncSubscriber();
        void setMsgarrvdCallback(const msgarrvd_t& msgarrvd);
        void subscribe(const std::string& topicFilter);
        
    private:
        static void connlost(void *context, char *cause);
        static int _msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
        msgarrvd_t m_msgarrvd;
    };
}
#endif /* ASYNCSUBSCRIBER_H */


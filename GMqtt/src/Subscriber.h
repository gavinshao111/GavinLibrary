/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Subscriber.h
 * Author: 10256
 *
 * Created on 2017年3月22日, 上午11:43
 */

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include "Client.h"
namespace gmqtt {

    
    class Subscriber : public Client {
    public:
        virtual ~Subscriber();
        void connect(const bool& ssl, const size_t& timeout);
        void subscribe(const std::string& topicFilter);
        
    protected:
        Subscriber(const std::string& serverURI,
                const std::string& clientId,
                const std::string& username,
                const std::string& password);
        //    Subscriber(const Subscriber& orig);
        std::string m_topicFilter;
        bool m_ssl;
        size_t m_connectTimeout;
        
        static void Free(void *topicName, MQTTClient_message* message);
    };
}
#endif /* SUBSCRIBER_H */


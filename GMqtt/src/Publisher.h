/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Publisher.h
 * Author: 10256
 *
 * Created on 2017年3月21日, 下午5:18
 */

#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "Client.h"

namespace gmqtt {

    class Publisher : public Client {
    public:
        Publisher(const std::string& serverURI,
                const std::string& clientId,
                const std::string& username,
                const std::string& password);
//        PublishClient(const PublishClient& orig);
        virtual ~Publisher();
        void publish(const std::string& tpc, const bytebuf::ByteBuffer& payload);
        void publish(const std::string& tpc, const bytebuf::ByteBuffer& payload, const size_t& offset, const size_t& size);
    private:
        
        MQTTClient_message m_pubmsg;
        MQTTClient_deliveryToken m_token;
        
    };
}

#endif /* PUBLISHER_H */


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SyncSubscriber.cpp
 * Author: 10256
 * 
 * Created on 2017年3月24日, 下午1:27
 */

#include "SyncSubscriber.h"
#include <iostream>
using namespace gmqtt;
using namespace std;
SyncSubscriber::SyncSubscriber(const string& serverURI,
        const string& clientId,
        const string& username,
        const string& password) : 
Subscriber(serverURI, clientId, username, password) {
}

//SyncSubscriber::SyncSubscriber(const SyncSubscriber& orig) {
//}

SyncSubscriber::~SyncSubscriber() {
}

bool SyncSubscriber::receive(bytebuf::ByteBuffer& out_data, const size_t& timeout) {
    int rc;
    MQTTClient_message* message;
    int topicLen;
    char* topicName = NULL;

    for (;;) {
        message = NULL;
        rc = MQTTClient_receive(m_client, &topicName, &topicLen, &message, 1000 * timeout);
        if (MQTTCLIENT_SUCCESS == rc || MQTTCLIENT_TOPICNAME_TRUNCATED == rc) {
            if (!message)
                return false;

            if (message->payloadlen > out_data.remaining()) {
                m_stream.clear();
                m_stream << "Subscriber::receive(): out_data space too small, payload length: " << message->payloadlen;
                Free(topicName, message);
                throw runtime_error(m_stream.str());
            }
            out_data.put((uint8_t*)message->payload, 0, message->payloadlen);
            Free(topicName, message);
            return true;
        }

        cout << "[WARN] Subscriber::receive(): MQTTClient_receive return " << rc << ". Reconnecting..." << endl;
        Client::connect(m_ssl, m_connectTimeout);
        subscribe(m_topicFilter);
        //    m_stream.clear();
        //    m_stream << "Subscriber::receive(): MQTTClient_receive return " << rc;
        //    throw runtime_error(m_stream.str());
    }
}


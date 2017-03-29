/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Subscriber.cpp
 * Author: 10256
 * 
 * Created on 2017年3月22日, 上午11:43
 */

#include "Subscriber.h"
#include <iostream>
#include <exception>

using namespace gmqtt;
using namespace std;

Subscriber::Subscriber(const string& serverURI,
        const string& clientId,
        const string& username,
        const string& password) :
Client(serverURI, clientId, username, password) {
}

//Subscriber::Subscriber(const Subscriber& orig) {
//}

Subscriber::~Subscriber() {
}

void Subscriber::subscribe(const string& topicFilter) {
    if (topicFilter.length() == 0 || topicFilter.at(0) != '/')
        throw runtime_error("Subscriber::subscribe(): topicFilter needs start with '/'");
    
    int rc;
    m_topicFilter = topicFilter;
    //    MQTTClient_setCallbacks(m_client, this, connlost, _msgarrvd, NULL);
    if (rc = MQTTClient_subscribe(m_client, m_topicFilter.c_str(), m_qos) != MQTTCLIENT_SUCCESS) {
        m_stream.clear();
        m_stream << "Subscriber::subscribe(): subscribe fail ,return " << rc;
        throw runtime_error(m_stream.str());
    }
}

// 隐藏 Client::connect(), 并不会有 基类指针指向子类对象，然后调用 connect 的情况，所以没必要声明 Client::connect 为 virtual

void Subscriber::connect(const bool& ssl, const size_t& timeout) {
    m_ssl = ssl;
    m_connectTimeout = timeout;
    Client::connect(m_ssl, m_connectTimeout);
}

void Subscriber::Free(void *topicName, MQTTClient_message* message) {
    if (topicName)
        MQTTClient_free(topicName);
    if (message)
        MQTTClient_freeMessage(&message);
}
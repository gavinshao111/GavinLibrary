/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AsyncSubscriber.cpp
 * Author: 10256
 * 
 * Created on 2017年3月24日, 上午10:52
 */

#include "AsyncSubscriber.h"
#include <iostream>
#include <exception>

using namespace gmqtt;
using namespace std;

AsyncSubscriber::AsyncSubscriber(const string& serverURI,
        const string& clientId,
        const string& username,
        const string& password) :
Subscriber(serverURI, clientId, username, password),
m_msgarrvd(0) {
}

//AsyncSubscriber::AsyncSubscriber(const AsyncSubscriber& orig) {
//}

AsyncSubscriber::~AsyncSubscriber() {
}

void AsyncSubscriber::setMsgarrvdCallback(const msgarrvd_t& msgarrvd) {
    m_msgarrvd = msgarrvd;
    MQTTClient_setCallbacks(m_client, this, connlost, _msgarrvd, NULL);
}

int AsyncSubscriber::_msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    if (NULL == context)
        throw runtime_error("AsyncSubscriber::_msgarrvd(): IllegalArgument context");
    if (NULL == message->payload || 1 > message->payloadlen)
        throw runtime_error("AsyncSubscriber::_msgarrvd(): Illegal payload");

    AsyncSubscriber* asyncSubscriber = (AsyncSubscriber*) context;
    string msg((char*) message->payload, message->payloadlen);
    Free(topicName, message);
    // if exception throwed from m_msgarrvd, there's no business with me, so i will not catch an exception.
    (*asyncSubscriber->m_msgarrvd)(msg);

    return 1;
}

void AsyncSubscriber::connlost(void *context, char *cause) {
    cout << "[WARN] Subscriber::connlost(): MQTT connection lost, cause: " << cause
            << "\nreconnecting..." << endl;
    if (NULL == context)
        throw runtime_error("AsyncSubscriber::connlost(): IllegalArgument context");
    AsyncSubscriber* asyncSubscriber = (AsyncSubscriber*) context;
    asyncSubscriber->connect(asyncSubscriber->m_ssl, asyncSubscriber->m_connectTimeout);
    asyncSubscriber->subscribe(asyncSubscriber->m_topicFilter);
}

void AsyncSubscriber::subscribe(const string& topicFilter) {
    if (0 == m_msgarrvd)
        throw runtime_error("AsyncSubscriber::subscribe(): you need call AsyncSubscriber::setMsgarrvdCallback() first");
    
    Subscriber::subscribe(topicFilter);
}




/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Publisher.cpp
 * Author: 10256
 * 
 * Created on 2017年3月21日, 下午5:18
 */

#include "Publisher.h"

using namespace gmqtt;
using namespace std;
using namespace bytebuf;

Publisher::Publisher(const string& serverURI,
                const string& clientId,
                const string& username,
                const string& password) :
Client(serverURI, clientId, username, password),
m_pubmsg((MQTTClient_message)MQTTClient_message_initializer) {
}

//PublishClient::PublishClient(const PublishClient& orig) {
//}

Publisher::~Publisher() {
}

void Publisher::publish(const std::string& tpc, const ByteBuffer& payload, const size_t& offset, const size_t& size) {
    if (payload.remaining() < offset + size)
        throw runtime_error("PublishClient::publish(): Illegal Argument");
    int rc;
    m_pubmsg.payload = (void *)(payload.array() + payload.position() + offset);
    m_pubmsg.payloadlen = size;
    m_pubmsg.qos = m_qos;
    m_pubmsg.retained = 0;
    if (rc = MQTTClient_publishMessage(m_client, tpc.c_str(), &m_pubmsg, &m_token) != MQTTCLIENT_SUCCESS) {
        m_stream.clear();
        m_stream << "publishMessage fail, return " << rc;
        throw runtime_error(m_stream.str());
    }
}
void Publisher::publish(const string& tpc, const ByteBuffer& payload) {
    publish(tpc, payload, 0, payload.remaining());
}
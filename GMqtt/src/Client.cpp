/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Client.cpp
 * Author: 10256
 * 
 * Created on 2017年3月21日, 下午3:35
 */

#include "Client.h"
#include <exception>
#include <iostream>
using namespace gmqtt;
using namespace std;
using namespace bytebuf;

Client::Client(const string& serverURI,
        const string& clientId,
        const string& username,
        const string& password
        ) :
//m_serverURI(serverURI),
//m_clientId(clientId),
m_username(username),
m_passwd(password),
        m_qos(1),
m_conn_opts((MQTTClient_connectOptions) MQTTClient_connectOptions_initializer),
m_ssl_opts((MQTTClient_SSLOptions) MQTTClient_SSLOptions_initializer),
        m_pubmsg((MQTTClient_message)MQTTClient_message_initializer),
        m_msgarrvd(0)
 {
    if (0 == serverURI.length() & clientId.length() & username.length() & password.length())
        throw runtime_error("Client::Client(): Illegal arguement");

    MQTTClient_create(&m_client, serverURI.c_str(), clientId.c_str(),
            MQTTCLIENT_PERSISTENCE_NONE, NULL);
    m_conn_opts.username = m_username.c_str();
    m_conn_opts.password = m_passwd.c_str();
    m_conn_opts.keepAliveInterval = 20;
    m_conn_opts.cleansession = 1;
    //    m_conn_opts = MQTTClient_connectOptions_initializer;
}

//Client::Client(const Client& orig) {
//}

Client::~Client() {
    MQTTClient_destroy(&m_client);
}

void Client::connect(const bool& ssl, const size_t& timeout) {
    int rc;
    
    m_ssl = ssl;
    m_conn_opts.ssl = ssl ? &m_ssl_opts : NULL;
    m_conn_opts.connectTimeout = timeout;
    rc = MQTTClient_connect(m_client, &m_conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        MQTTClient_destroy(&m_client);
        m_stream.clear();
        m_stream << "Client::connect(): Failed to connect, return " << rc;
        throw runtime_error(m_stream.str());
    }
}

void Client::disconnect() {
    MQTTClient_disconnect(m_client, 100);
}

void Client::setSslOption(const string& pathOfServerPublicKey, const string& pathOfPrivateKey) {
    if (0 == pathOfServerPublicKey.length() & pathOfPrivateKey.length())
        throw runtime_error("Client::setSslOption(): Illegal arguement");
    m_serverPublicKeyPath = pathOfServerPublicKey;
    m_privateKeyPath = pathOfPrivateKey;
    m_ssl_opts.trustStore = m_serverPublicKeyPath.c_str();
    m_ssl_opts.keyStore = m_serverPublicKeyPath.c_str();
    m_ssl_opts.privateKey = m_privateKeyPath.c_str();   
    m_ssl_opts.enableServerCertAuth = 0;
}

void Client::setMsgarrvdCallback(const msgarrvd_t& msgarrvd) {
    m_msgarrvd = msgarrvd;
    MQTTClient_setCallbacks(m_client, this, connlost, _msgarrvd, NULL);
}

void Client::subscribe(const string& topicFilter) {
    if (topicFilter.length() == 0 || topicFilter.at(0) != '/')
        throw runtime_error("Client::subscribe(): topicFilter needs start with '/'");
    
    int rc;
    m_topicFilter = topicFilter;
    //    MQTTClient_setCallbacks(m_client, this, connlost, _msgarrvd, NULL);
    if (rc = MQTTClient_subscribe(m_client, m_topicFilter.c_str(), m_qos) != MQTTCLIENT_SUCCESS) {
        m_stream.clear();
        m_stream << "Subscriber::subscribe(): subscribe fail ,return " << rc;
        throw runtime_error(m_stream.str());
    }
}

int Client::_msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    if (NULL == context)
        throw runtime_error("Client::_msgarrvd(): IllegalArgument context");
    if (NULL == message->payload || 1 > message->payloadlen)
        throw runtime_error("Client::_msgarrvd(): Illegal payload");

    Client* asyncSubscriber = (Client*) context;
    string msg((char*) message->payload, message->payloadlen);
    Free(topicName, message);
    // if exception throwed from m_msgarrvd, there's no business with me, so i will not catch an exception.
    (*asyncSubscriber->m_msgarrvd)(msg);

    return 1;
}

void Client::connlost(void *context, char *cause) {
    cout << "[WARN] Client::connlost(): MQTT connection lost, cause: " << cause
            << "\nreconnecting..." << endl;
    if (NULL == context)
        throw runtime_error("Client::connlost(): IllegalArgument context");
    Client* asyncSubscriber = (Client*) context;
    asyncSubscriber->connect(asyncSubscriber->m_ssl, asyncSubscriber->m_conn_opts.connectTimeout);
    asyncSubscriber->subscribe(asyncSubscriber->m_topicFilter);
}

void Client::Free(void *topicName, MQTTClient_message* message) {
    if (topicName)
        MQTTClient_free(topicName);
    if (message)
        MQTTClient_freeMessage(&message);
}

bool Client::receive(bytebuf::ByteBuffer& out_data, const size_t& timeout) {
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
                m_stream << "Client::receive(): out_data space too small, payload length: " << message->payloadlen;
                Free(topicName, message);
                throw runtime_error(m_stream.str());
            }
            out_data.put((uint8_t*)message->payload, 0, message->payloadlen);
            Free(topicName, message);
            return true;
        }

        cout << "[WARN] Client::receive(): MQTTClient_receive return " << rc << ". Reconnecting..." << endl;
        connect(m_ssl, m_conn_opts.connectTimeout);
        subscribe(m_topicFilter);
        //    m_stream.clear();
        //    m_stream << "Subscriber::receive(): MQTTClient_receive return " << rc;
        //    throw runtime_error(m_stream.str());
    }
}

void Client::publish(const std::string& tpc, const ByteBuffer& payload, const size_t& offset, const size_t& size) {
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
void Client::publish(const string& tpc, const ByteBuffer& payload) {
    publish(tpc, payload, 0, payload.remaining());
}
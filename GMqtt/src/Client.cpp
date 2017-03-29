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
using namespace gmqtt;
using namespace std;

Client::Client(const string& serverURI,
        const string& clientId,
        const string& username,
        const string& password
        ) :
//m_serverURI(serverURI),
//m_clientId(clientId),
m_username(username),
m_passwd(password),
m_conn_opts((MQTTClient_connectOptions) MQTTClient_connectOptions_initializer),
m_ssl_opts((MQTTClient_SSLOptions) MQTTClient_SSLOptions_initializer),
m_qos(1) {
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
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Client.h
 * Author: 10256
 *
 * Created on 2017年3月21日, 下午3:35
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <sstream>
#include "ByteBuffer.h"

extern "C" {
#include "MQTTClient.h"
}

namespace gmqtt {

    class Client {
    public:
        virtual ~Client();
        
        void connect(const bool& ssl, const size_t& timeout);
        void disconnect();
        void setSslOption(const std::string& pathOfServerPublicKey, const std::string& pathOfPrivateKey);
    protected:
        Client(const std::string& serverURI,
                const std::string& clientId,
                const std::string& username,
                const std::string& password
//                const int& qos,
//                const size_t& keepAliveInterval,
//                const bool& cleansession
                );
        
        
//        Client(const Client& orig);
//        std::string m_serverURI;
//        std::string m_clientId;
        std::string m_username;
        std::string m_passwd;
        int m_qos;
        
        MQTTClient m_client;
        MQTTClient_connectOptions m_conn_opts;
        MQTTClient_SSLOptions m_ssl_opts;
        std::stringstream m_stream;
        std::string m_serverPublicKeyPath;
        std::string m_privateKeyPath;
        
    };

}

#endif /* CLIENT_H */


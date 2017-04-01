/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GMqttClient.h
 * Author: 10256
 *
 * Created on 2017年3月21日, 下午3:35
 */

#ifndef GMQTTCLIENT_H
#define GMQTTCLIENT_H

#include <string>
#include <sstream>
#include <boost/function.hpp>

#include "ByteBuffer.h"

extern "C" {
#include "MQTTClient.h"
}

namespace gmqtt {
//    typedef void (*MsgArrvdHandler)(const std::string&, const std::string&);
    typedef boost::function<void(const std::string&, bytebuf::ByteBuffer&)> MsgArrvdHandlerF;

    class GMqttClient {
    public:
        GMqttClient(const std::string& serverURI,
                const std::string& clientId,
                const std::string& username,
                const std::string& password
                //                const int& qos,
                //                const size_t& keepAliveInterval,
                //                const bool& cleansession
                );
        virtual ~GMqttClient();

        void connect(const bool& ssl, const size_t& timeout);
        void disconnect();
        void setSslOption(const std::string& pathOfServerPublicKey, const std::string& pathOfPrivateKey);
        
        /**
         * 
         * @param msgarrvd there's no exception expected throwed from msgarrvd, user should catch an exception in msgarrvd. msgarrvd is like\
         * void func(const std::string& topic, bytebuf::ByteBuffer& payload). payload is read only.
         */
        void setMsgarrvdCallback(const MsgArrvdHandlerF& msgarrvd);
        void subscribe(const std::string& topicFilter);
        
        void publish(const std::string& tpc, const bytebuf::ByteBuffer& payload);
        void publish(const std::string& tpc, const bytebuf::ByteBuffer& payload, const size_t& offset, const size_t& size);
        /**
         * 
         * @param out_data payload will be wrote into this argument, exception throwed if in_data doesn't have enough space.
         * @param timeout The length of time to wait for a message in seconds.
         * @return true if successful, false if timeout
         */
        bool receive(bytebuf::ByteBuffer& out_data, const size_t& timeout);

    protected:


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
        // Subscriber
        std::string m_topicFilter;
        bool m_ssl;      
        // AsyncSubscriber
//        MsgArrvdHandler m_msgarrvd;
        MsgArrvdHandlerF m_msgarrvdF;
        
        MQTTClient_message m_pubmsg;
        MQTTClient_deliveryToken m_token;
        
        
        static void Free(void *topicName, MQTTClient_message* message);
        static void connlost(void *context, char *cause);
        static int _msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    };

}

#endif /* GMQTTCLIENT_H */


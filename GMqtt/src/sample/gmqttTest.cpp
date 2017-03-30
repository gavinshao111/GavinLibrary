/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <exception>
#include <iostream>
#include <thread>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include "../Client.h"
//#include "../Publisher.h"
//#include "../AsyncSubscriber.h"
#include "ByteBuffer.h"

using namespace std;
using namespace gmqtt;
using namespace bytebuf;

static void AMsgarrvd(const string& payload);
void publishTestTask(const bool& ssl);

void asyncSubscribeMq(const bool& ssl);
void PublisherTest(const bool& ssl);
const static string TcpServerAddr = "tcp://120.26.86.124:1883";
const static string SslServerAddr = "ssl://120.26.86.124:8883";
const static string ClientId = "cppTest3";
const static string ClientIdSub = "gmqttTestSub";
const static string Username = "easydarwin";
const static string Passwd = "123456";
const static string Topic = "/mqttTest";
const static string Payload = "hello world";
const static string pathOfPrivateKey = "/mnt/hgfs/ShareFolder/ED/emqtt.key";
const static string pathOfServerPublicKey = "/mnt/hgfs/ShareFolder/ED/emqtt.pem";

mutex mtxForMsgArrived;
condition_variable msgArrived;

int main(int argc, char** argv) {
    bool ssl = true;
    thread subThread(asyncSubscribeMq, ssl);
    sleep(1);
    thread pubThread(publishTestTask, ssl);
    subThread.join();
    pubThread.join();
    
    cout << "ssl done." << endl;
    
    ssl = false;
    thread subThread2(asyncSubscribeMq, ssl);
    sleep(1);
    thread pubThread2(publishTestTask, ssl);
    subThread2.join();
    pubThread2.join();
    
    cout << "tcp done." << endl;
    
    return 0;
}

static void AMsgarrvd(const string& payload) {
    if (0 != payload.compare(Payload))
        cout << "%TEST_FAILED% time=0 testname=asyncSubscribeMq (gmqttTest) message="
            << "async subscriber receive MQ: " << payload << ", which is not match with publisher sent." << endl;
    else
        cout << "msg matched." << endl;

    unique_lock<mutex> lk(mtxForMsgArrived);
    msgArrived.notify_one();
}

void publishTestTask(const bool& ssl) {
    string url = ssl ? SslServerAddr : TcpServerAddr;
    boost::shared_ptr<Client> publisher = boost::make_shared<Client>(url, ClientId, Username, Passwd);
//    Client* publisher = new Client(url, ClientId, Username, Passwd);
    ByteBuffer* payload = ByteBuffer::allocate(100);
    payload->put(Payload);
    payload->flip();
    try {
        if (ssl)
            publisher->setSslOption(pathOfServerPublicKey, pathOfPrivateKey);
        publisher->connect(ssl, 10);
        cout << "publisher connected" << endl;
//        for (int i = 0; i < 1; i++, sleep(1))
            publisher->publish(Topic, *payload);
        cout << "publisher published" << endl;

        publisher->disconnect();
    } catch (const exception& e) {
        cout << "error: " << e.what() << endl;
    }
    
    payload->freeMemery();
    delete payload;
//    delete publisher;
    
    cout << "publisher done" << endl;
}

void asyncSubscribeMq(const bool& ssl) {
    string url = ssl ? SslServerAddr : TcpServerAddr;
    boost::shared_ptr<Client> subscriber = boost::make_shared<Client>(url, ClientIdSub, Username, Passwd);
//    Client* subscriber = new Client(url, ClientIdSub, Username, Passwd);
    try {
        subscriber->setMsgarrvdCallback(&AMsgarrvd);
        if (ssl)
            subscriber->setSslOption(pathOfServerPublicKey, pathOfPrivateKey);
        subscriber->connect(ssl, 10);
        cout << "subscriber connected" << endl;
        subscriber->subscribe(Topic);
        cout << "subscriber subscribed" << endl;

        unique_lock<mutex> lk(mtxForMsgArrived);

#ifdef __GXX_EXPERIMENTAL_CXX0X__  // c++0x wait_for return false if timeout
        if (!msgArrived.wait_for(lk, chrono::seconds(5)))
#else // c11
        if (cv_status::timeout == msgArrived.wait_for(lk, chrono::seconds(5)))
#endif        
            cout << "%TEST_FAILED% time=0 testname=AsyncMQTest (gmqttTest) message="
                << "async subscriber wait for MQ timeout" << endl;

        else
            cout << "subscriber test passed" << endl;
        sleep(1);   // client must disconnect first, otherwise segmentation fault.
        subscriber->disconnect();
        cout << "subscriber disconnected" << endl;

    } catch (const exception& e) {
        cout << "error: " << e.what() << endl;
    }
//    delete subscriber;
    cout << "subscriber done" << endl;
}
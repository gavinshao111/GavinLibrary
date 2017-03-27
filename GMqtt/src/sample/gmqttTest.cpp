/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <exception>
#include <iostream>
#include <thread>
#include "../Publisher.h"
#include "../AsyncSubscriber.h"
#include "ByteBuffer.h"

using namespace std;
using namespace gmqtt;
using namespace bytebuf;

static void AMsgarrvd(const string& payload);
void publishTestTask(const bool& ssl);
void asyncSubscribeMq(const bool& ssl);

//const static string TcpServerAddr = "tcp://120.26.86.124:1883";
const static string SslServerAddr = "ssl://120.26.86.124:8883";
const static string ClientId = "cppTest3";
const static string ClientIdSub = "gmqttTestSub";
const static string Username = "easydarwin";
const static string Passwd = "123456";
const static string Topic = "/mqttTest";
const static string Payload = "hello world";
const static string pathOfPrivateKey = "/mnt/hgfs/ShareFolder/ED/emqtt.key";
const static string pathOfServerPublicKey = "/mnt/hgfs/ShareFolder/ED/emqtt.key";
mutex mtxForMsgArrived;
condition_variable msgArrived;

int main(int argc, char** argv) {
    bool ssl = true;
    thread subThread(asyncSubscribeMq, ssl);
    sleep(1);
    thread pubThread(publishTestTask, ssl);
    subThread.join();
    pubThread.join();


    cout << "done." << endl;
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
    Publisher* publisher = new Publisher(SslServerAddr, ClientId, Username, Passwd);
    ByteBuffer* payload = ByteBuffer::allocate(100);
    payload->put(Payload);
    payload->flip();
    try {
        publisher->setSslOption(pathOfServerPublicKey, pathOfPrivateKey);
        publisher->connect(true, 10);
        for (int i = 0; i < 10; i++, sleep(1))
            publisher->publish(Topic, *payload);

        publisher->disconnect();
    } catch (const exception& e) {
        cout << "error: " << e.what() << endl;
    }
    payload->freeMemery();
    delete payload;
    delete publisher;
}

void asyncSubscribeMq(const bool& ssl) {
    AsyncSubscriber* subscriber = new AsyncSubscriber(SslServerAddr, ClientIdSub, Username, Passwd);
    try {
        subscriber->setMsgarrvdCallback(&AMsgarrvd);
        subscriber->setSslOption(pathOfServerPublicKey, pathOfPrivateKey);
        subscriber->connect(true, 10);
        subscriber->subscribe(Topic);

        unique_lock<mutex> lk(mtxForMsgArrived);

#ifdef __GXX_EXPERIMENTAL_CXX0X__  // c++0x wait_for return false if timeout
        if (!msgArrived.wait_for(lk, chrono::seconds(5)))
#else // c11
        if (cv_status::timeout == msgArrived.wait_for(lk, chrono::seconds(10)))
#endif        
            cout << "%TEST_FAILED% time=0 testname=AsyncMQTest (gmqttTest) message="
                << "async subscriber wait for MQ timeout" << endl;


        subscriber->disconnect();

    } catch (const exception& e) {
        cout << "error: " << e.what() << endl;
    }
    delete subscriber;
}
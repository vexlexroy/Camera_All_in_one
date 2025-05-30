#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>
#include <boost/asio.hpp>

class NodeDelayMesurmente : public NodeBase{

public:
    GLuint texture = 0;

    std::unique_ptr<std::pair<cv::Mat, cv::Mat>> lastMsg = nullptr;
    std::shared_ptr<MessageBase> LastMessage;
    cv::Mat frame;

    char* items[3] = { "-1-", "-2-", "-3-" };
    int currentItem = 0;


    ImVec2 dragStartPos;
    ImVec2 dragEndPos;
    bool dragging = false;

    ImVec2 zoom_A = ImVec2(0.0f, 0.0f);
    ImVec2 zoom_B = ImVec2(1.0f, 1.0f);
    GLuint selectTexture = 0;
    bool isShiftPresed = false;
    bool freezeFrameFlag = false;

    long long int startTime = NULL;
    long long int currentTime = NULL;
    long long int lineTime = NULL;
    int lastBlobCount = NULL;
    int interval = NULL;
    int lastDelay = 0;
    int last_Delays[3] = {0,0,0}; //not in use
    int lastAvgDelay = 0; //not in use
    int numOfftests = 0; //not in use
    long long int test_timer = NULL;
    int test_period = 100; //seconds


    int hue[2] = {255, 0};
    int sat[2] = {255, 0};
    int val[2] = {255, 0};
    int roix[2] = {0,640}; // REGION OF INTEREST (testing area for led)
    int roiy[2] = {0,480};
    int area[2] = {300, 400}; //area of blob to be detected
    int maxTime = 5000; // time witch stops delay test in ms
    int timeOut = 10000; // time witch stops test if no msg from arduino comes

    bool roiSelected = false;
    bool roiOn = false;
    bool ledOn = false;
    bool isLedOn = false;
    bool showBlob = false;
    bool startTest = false;
    bool sentRequest = false;
    bool response = false;
    bool isTesting = false;
    bool useAvg = false; //always fals (not in use)
    bool isPeriodicTest = false;
    bool syncedTest = false;
    bool freetotest = true;
    bool initiator = false;

    char port[128] = "COM_"; // ARDUINO port
    std::string using_port;
    int baudRate = 9600;
    boost::asio::io_context io;
    std::unique_ptr<boost::asio::serial_port> serial;

    


    NodeDelayMesurmente(int uniqueId);

    std::shared_ptr<NodeBase> createNewClassInstance(int uniqueId) override;

    std::vector<Enums::MessageType> getInMessageTypes() override;
    std::vector<Enums::MessageType> getOutMessageTypes() override;

    std::string getDescription() override;
    std::string getName() override;
    Enums::NodeType getType() override;
    void drawNodeParams() override;
    void drawNodeWork() override;

    int blobCount(cv::Mat image); //counts blobs on screan
    bool testDelay(cv::Mat image); //starts delay testing
    void initiateTest(cv::Mat image); //sets everything up for testing
    bool connect(std::string port, unsigned int baud); //conect to arduino serial
    void disconnect(); //disconects from serial
    bool isConnected(); //checks if serial is conected
    void sendData(std::string data); //sends string msg to serial
    bool isPortAvailable(std::string port);
    bool readData(); //reads data from serial

    void recieve(std::shared_ptr<MessageBase> message, int connectorId) override;
    //void connectionAdded(int connectorId, int connectionId) override;
    //void connectionRemoved(int connectorId, int connectionId) override;


};

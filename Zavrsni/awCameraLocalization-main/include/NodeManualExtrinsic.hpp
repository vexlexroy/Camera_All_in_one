#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include "FrameBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include "Message.hpp"
#include <imgui.h>
#include <string.h>

class NodeManualExtrinsic : public NodeBase{
public:
    char customFrameInputText[128] = "";


    bool captureFrames = false;
    GLuint texture1stCam = 0;
    GLuint texture2ndCam = 0;
    ImVec2 zoom1_A = ImVec2(0.0f, 0.0f);;
    ImVec2 zoom1_B = ImVec2(1.0f, 1.0f);;

    ImVec2 zoom2_A = ImVec2(0.0f, 0.0f);;
    ImVec2 zoom2_B = ImVec2(1.0f, 1.0f);;

    //Od koga i što...

    std::vector<std::pair<std::string, std::string>> selectedPair;
    std::pair<std::string, std::string> hoveredPair;
    std::unordered_map<std::string, ImVec4> camColor;
    bool selectAll = false;

    std::unordered_map<std::string, Structs::ExtrinsicPair> extrinsicPair; 

    std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> firstCamMsg = nullptr;
    std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> secondCamMsg = nullptr;

    std::pair<cv::Mat, cv::Mat> firstLastMsg;
    std::pair<cv::Mat, cv::Mat> secondLastMsg;

    bool firstCamShouldIntrinsicalyCorrect = false;
    bool secondCamShouldIntrinsicalyCorrect = false;

    bool shouldAutoChessboardCalibrate = false;
    int W=9;
    int H=7;

    bool showEpipolarLines = true;

    std::vector<std::pair<std::pair<cv::Mat, cv::Mat>, long long int>> camHistory;
    bool historyOfWhatCam = false; // false -> first cam, true -> second cam

    int matchIntervalInUs = 50000;

    bool isShiftPresed = false;
    bool freezeFrameFlag = false;


    std::vector<std::pair<ImVec2, ImVec2>> pairList;
    std::pair<ImVec2, ImVec2> pairNew;

    int closestPairIndex;

    bool shouldBeColorView = false;
    
    int customFrameProcedureCnt = -1;
    cv::Point2f custom_frame_origin_left;
    cv::Point2f custom_frame_point1_left;
    cv::Point2f custom_frame_point2_left;
    //everything else goes to rotation directrly...
    cv::Point3f custom_frame_translationVector;
    cv::Mat custom_frame_rotationMatrix = cv::Mat::zeros(3, 3, CV_32F);

    
    bool showtargetFrameCordSystem = false;
    double worldFrameAxisLen = 10;
    int cordinateFrameOption = 1;
    const char* cordinateFrameOptions[3] = { "static", "static-object", "dynamic" };

    bool calibrate_extrinsic_flag = false;


    bool showTriangulatedLines = false;
    bool showTriangulatedNumbers = false;
    int numbersFont = 5;

    float prob = 0.999;
    float threshold = 0.001; //0.003
    int maxIters = 1000;


    int hue1[2] = {-1, -1};
    int sat1[2] = {255, 0};
    int val1[2] = {255, 0};

    int hue2[2] = {-1, -1};
    int sat2[2] = {255, 0};
    int val2[2] = {255, 0};

    cv::Point2f cam1_2D;
    cv::Point2f cam2_2D;
    bool cam1_2D_updated = false;
    bool cam2_2D_updated = false;

    std::shared_ptr<FrameRelation> currFrameRelation = nullptr;
    std::shared_ptr<FrameBase> worldFrame = nullptr;


    std::unordered_map<std::string, std::pair<cv::Mat, GLuint>> incomingCamFrames;

    bool triggerExternalCalibration = false;
    std::pair<std::string, std::string> externalCalibrationPair;

    NodeManualExtrinsic(int uniqueId);

    std::shared_ptr<NodeBase> createNewClassInstance(int uniqueId) override;

    std::vector<Enums::MessageType> getInMessageTypes() override;
    std::vector<Enums::MessageType> getOutMessageTypes() override;

    std::string getDescription() override;
    std::string getName() override;
    Enums::NodeType getType() override;
    void drawNodeParams() override;
    void drawNodeWork() override;

    void recieve(std::shared_ptr<MessageBase> message, int connectorId) override;
    //void connectionAdded(int connectorId, int connectionId) override;
    //void connectionRemoved(int connectorId, int connectionId) override;
};

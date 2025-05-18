#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>

class NodeBlobCreator : public NodeBase{
public:
    GLuint texture = 0;
    std::unique_ptr<cv::Mat> rawPicture = nullptr;

    std::vector<std::pair<std::vector<cv::Point>, std::vector<std::vector<cv::Point>>>> orderedContours;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> contoursHierarchy;
    std::vector<std::pair<int, std::vector<int>>> myHierarchy;

    std::vector<std::pair<cv::Point2f, float>> circles;
    std::vector<std::pair<int, cv::RotatedRect>> rectangles;
    std::vector<std::pair<cv::Moments, std::vector<cv::Moments>>> moments;
    std::vector<Structs::BlobDescription> blobs;



    const char* contourMode[2] = {"RETR_EXTERNAL", "RETR_CCOMP"};
    int contourModeCurrent = 0;
    cv::RetrievalModes retrievalMode = cv::RetrievalModes::RETR_EXTERNAL;

    const char* contourAprox[4] = {"NONE", "SIMPLE", "TC89_L1", "TC89_KCOS"};
    int contourAproxCurrent = 1;
    cv::ContourApproximationModes aproxMode = cv::ContourApproximationModes::CHAIN_APPROX_NONE;


    ImVec4 colorStar = ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

    const char* objectCenterMethod[4] = {"arithmeticMiddle", "weightedArithmeticMiddle", "circleMiddle", "rectangleMiddle"};
    int objectCenterMethodCurrent = 2;
    bool isCenterHovered = false;
    ImVec4 colorCenter = ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);


    const char* objectSizeMethod[3] = {"pixelArea", "circleArea", "rectangleArea"};
    int objectSizeMethodCurrent = 0;
    bool isSizeHovered = false;
    bool isSizeEnabled = true;
    ImVec4 colorSize = ImVec4(150.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

    

    const char* objectDispersityMethod[3] = {"circleAreaPart", "rectangleAreaPart", "densityQuotient"};
    int objectDispersityMethodCurrent = 0;
    bool isDispersityHovered = false;
    bool isDispersityEnabled = false;
    ImVec4 colorDispersity = ImVec4(80.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

    bool isHuEnabled = false;
    bool isHuHovered = false;
    ImVec4 colorHu = ImVec4(0.0f / 255.0f, 50.0f / 255.0f, 255.0f / 255.0f, 200.0f / 255.0f);

    int maxBlobCnt = 1;
    float minBoundingRectArea = 20;
    float maxBoundingRectArea = 1000;

    

    NodeBlobCreator(int uniqueId);

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

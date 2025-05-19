#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>

class NodeArucoTracking : public NodeBase{
public:
    GLuint texture = 0;

    std::unique_ptr<std::pair<cv::Mat, cv::Mat>> lastMsg = nullptr;

    ImVec2 zoom_A = ImVec2(0.0f, 0.0f);
    ImVec2 zoom_B = ImVec2(1.0f, 1.0f);
    GLuint selectTexture = 0;
    bool isShiftPresed = false;
    bool freezeFrameFlag = false;

    std::vector<std::string> availableCameras;
    int selectedCameraIndex=0;
    std::string selectedCameraName = "";


    NodeArucoTracking(int uniqueId);

    std::shared_ptr<NodeBase> createNewClassInstance(int uniqueId) override;

    std::vector<Enums::MessageType> getInMessageTypes() override;
    std::vector<Enums::MessageType> getOutMessageTypes() override;

    std::string getDescription() override;
    std::string getName() override;
    Enums::NodeType getType() override;
    void drawNodeParams() override;
    void drawNodeWork() override;

    void recieve(std::shared_ptr<MessageBase> message, int connectorId) override;

    void drawDropdownSelector();
    void drawWorldSelector();
    void drawMainCamSelector();
    void calculateExtrinsicForParametars(std::string mainCam, std::string worldFrame);
    void saveExtrinsics(std::string fileName);
    void loadExtrinsics();
    cv::Mat sendArucoPositions(cv::Mat img, std::string camframe);
    //void connectionAdded(int connectorId, int connectionId) override;
    //void connectionRemoved(int connectorId, int connectionId) override;
};

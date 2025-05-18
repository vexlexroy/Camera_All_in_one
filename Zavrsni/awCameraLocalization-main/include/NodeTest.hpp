#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>

class NodeTest : public NodeBase{
public:
    GLuint texture = 0;

    std::unique_ptr<std::pair<cv::Mat, cv::Mat>> lastMsg = nullptr;
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



    int hue[2] = {255, 0};
    int sat[2] = {255, 0};
    int val[2] = {255, 0};


    NodeTest(int uniqueId);

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

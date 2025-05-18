#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>

class NodeBackgroundSubstraction : public NodeBase{
public:
    GLuint texture = 0;
    std::unique_ptr<std::pair<cv::Mat, cv::Mat>> filteredMsg = nullptr;
    std::unique_ptr<cv::Mat> backgroundImage;

    const char* typesStrings[2] = {"MOG2", "KNN"};
    int typeCurrent = 0;

    

    NodeBackgroundSubstraction(int uniqueId);

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

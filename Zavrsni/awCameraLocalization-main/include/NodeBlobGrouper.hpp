#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include "Message.hpp"

#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>
#include <boost/circular_buffer.hpp>
#include <unordered_map>
#include <implot.h>

class NodeBlobGrouper : public NodeBase{
public:
    int blobGroupsTimeDiffMs = 100;

    int viewLastMs = 2000;
    //long long int timestampRealTimeCnt = 0;
    float points[10000];

    ImVec4 defaultColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    bool calibrationActive = false;
    int calibrationMinDistance = 30;
    int calibrationPointsNum = 20;

    ImVec4 calibrationColor = ImVec4(0.78f, 0.78f, 0.35f, 1.0f);
    bool estimationActive = false;
    ImVec4 estimationColor = ImVec4(0.45f, 0.78f, 0.34f, 1.0f);

    std::vector<std::pair<std::string, std::string>> selectedPair;
    std::pair<std::string, std::string> hoveredPair;
    std::unordered_map<std::string, ImVec4> camColor;
    bool selectAll = false;
    //std::string plotHovered = "";
    //ImPlotPoint plotHoverPosition;

    cv::Mat rotationMatrix;
    cv::Mat translationMatrix;
    float x_point = 0;
    float y_point = 0;
    float z_point = 0;

    
    std::unordered_map<std::string, boost::circular_buffer<std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>>> data;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ImVec2>>> extrinsicCalibrationPairs;

    NodeBlobGrouper(int uniqueId);

    std::shared_ptr<NodeBase> createNewClassInstance(int uniqueId) override;

    std::vector<Enums::MessageType> getInMessageTypes() override;
    std::vector<Enums::MessageType> getOutMessageTypes() override;

    std::string getDescription() override;
    std::string getName() override;
    Enums::NodeType getType() override;
    void drawNodeParams() override;
    void drawNodeWork() override;

    void recieve(std::shared_ptr<MessageBase> message, int connectorId) override;

};

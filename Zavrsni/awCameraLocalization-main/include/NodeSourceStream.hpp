#pragma once
#include "NodeSource.hpp"
#include "Enums.hpp"
#include "NodeBase.hpp"
#include "FrameCam.hpp"


#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>




class NodeSourceStream: public NodeSource{
    public:
        GLuint texture = 0;
        cv::Mat frame;
        float exposureValue = -4;
        float whiteBalanceValue = -4;
        int n = 20; // Number of spikes
        int radius_min = 50; // Minimum radius
        int radius_max = 100;
        int pass = 0;
        // int resolution[2] = {640,480};
        bool changedres=false;

        bool intrinsicAndDistortionButtonFlag = false;
        std::vector<std::vector<cv::Point2f>> calibrationPointsVector;
        int minCalibPictures = 15;
        int streamLagInMilisecond = 0;
        bool shouldUndistort = false;

        int ocupiedDShowNum = -1;
        //char streamString[128] = "";
        
        std::string streamFullName;


        char streamInputBuffer[256] = "";
        char nicknameBuffer[256] = "";
        
        Enums::FrameCamState frameCamState = Enums::FrameCamState::DISCONNECTED;
        std::shared_ptr<FrameCam> openedStream;

        
        int boardH=0;
        int boardW=0;
        float boardSQR=0.0f;

        NodeSourceStream(int uniqueId);
        std::shared_ptr<NodeBase> createNewClassInstance(int uniqueId) override;
        std::vector<Enums::MessageType> getInMessageTypes() override;
        std::vector<Enums::MessageType> getOutMessageTypes() override;
        std::string getDescription() override;
        std::string getName() override;
        Enums::NodeType getType() override;
        void drawNodeParams() override;
        void drawNodeWork() override;
        void threadLoop() override;

        void NodeSourceStream::connectionAdded(int connectorId, int connectionId) override;

        void NodeSourceStream::connectionRemoved(int connectorId, int connectionId) override;

        void recieve(std::shared_ptr<MessageBase> message, int connectorId) override;
};
#pragma once

#include "Enums.hpp"
#include "NodeBase.hpp"
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>

class NodeArucoTracking : public NodeBase{
public:
    GLuint texture = 0;

    std::unique_ptr<std::pair<cv::Mat, cv::Mat>> lastMsg = nullptr;

    ImVec2 zoom_A = ImVec2(0.0f, 0.0f);
    ImVec2 zoom_B = ImVec2(1.0f, 1.0f);
    GLuint selectTexture = 0;
    bool isShiftPresed = false;
    bool freezeFrameFlag = false;
    bool showPositiontxt = true;

    std::vector<std::string> availableCameras;
    std::vector<std::string> availableWorlds;
    int selectedCameraIndex=0;
    std::string selectedCameraName = "";
    int selectedWorldIndex=0;
    std::string selectedWorld = "";
    //ArucoTracking
    cv::aruco::Dictionary arucoDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    float markerSize = 3.8; //cm
    float fontsize = 0.8;
    //UDP comunication
    boost::asio::io_context asio_io;
    std::unique_ptr<boost::asio::ip::udp::socket> socket;
    boost::asio::ip::udp::endpoint conection;
    std::string ip;
    char ipBuffer[256] = {0};
    int port = 12345;
    bool initialised = false;
    bool sending = false;
    bool showWorld=false;




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
    //void drawMainCamSelector();
    std::shared_ptr<FrameRelation> calculateExtrinsicForParametars(std::string mainCam, std::string worldFrame);
    void getConCams();
    void getValWorlds();
    cv::Mat arucoPositions(cv::Mat img, std::string camframe, std::string worldFrame, std::vector<cv::Mat>& allposes, std::vector<int>& allids, nlohmann::json& jsonData);
    void sendData(std::string cam, long long int tstamp, long long int delay, long long int delay2, nlohmann::json markerData);
    cv::Mat NodeArucoTracking::drawworldFrame(cv::Mat img, std::string cam, std::string world);

    void rotationMatrixToEulerAngles(const cv::Mat& R, double& roll, double& pitch, double& yaw);
    void rotationMatrixToQuaternion(const cv::Mat& R, double& qw,double& qx,double& qy,double& qz);
    void drawColoredSegment(cv::Mat& img, const std::string& text, const cv::Scalar& color, cv::Point& position, double fontsize, int thickness, int* x_offset);
    //void connectionAdded(int connectorId, int connectionId) override;
    //void connectionRemoved(int connectorId, int connectionId) override;
};

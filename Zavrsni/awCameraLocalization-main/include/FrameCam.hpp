#pragma once
#include "FrameBase.hpp"
#include "Enums.hpp"
#include "Structs.hpp"
#include <string>
//#include <opencv2/opencv.hpp>

class FrameCam : public FrameBase{
public:
    std::string frameUrlName;
    int streamLagInMilisecond;
    bool isConnected;

    Structs::ColorThresholdParams colorThresholdParams;

    Structs::IntrinsicCamParams intrinsicParams;
    //std::vector<Ex>


    FrameCam(std::string frameUrlName, std::string frameNickName, int streamLagInMilisecond, Structs::IntrinsicCamParams intrinsicParams, Structs::ColorThresholdParams colorThresholdParams);

private:

};

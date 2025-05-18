#include "FrameCam.hpp"
#include "Enums.hpp"
#include "Structs.hpp"

FrameCam::FrameCam(std::string frameUrlName, std::string frameNickName, int streamLagInMilisecond, Structs::IntrinsicCamParams intrinsicParams, Structs::ColorThresholdParams colorThresholdParams): FrameBase(frameNickName){
    this->streamLagInMilisecond = streamLagInMilisecond;
    this->frameUrlName = frameUrlName;
    this->isConnected = false;
    this->intrinsicParams = intrinsicParams;
    this->colorThresholdParams = colorThresholdParams;
}
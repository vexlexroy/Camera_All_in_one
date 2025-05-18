#pragma once
#include "FrameRelation.hpp"

#include <vector>
#include <memory>
#include <string>
//#include <opencv2/opencv.hpp>

class FrameRelation;

class FrameBase {
public:
    std::string frameNickName;
    //cv::Mat test;
    FrameBase(std::string frameNickName);
private:

};

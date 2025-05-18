#pragma once
#include <imgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/matx.hpp>
#include <glad/glad.h>

namespace Structs{
    struct BlobDescription{
        ImVec2 middle = ImVec2(0, 0);
        float size = 0.0f;
        float dispersity = 0.0f;
        double huMoments[7] = {0.0};
    };

    struct ViewParams{
        bool isShowWork = true;
    };

    struct ZoomParams{
        float scaleFactor = 0.5;
        float scaleFactorMin = 0.1;
        float scaleFactorMax = 2;
        float scaleFactorSpeed = 0.05;
    };

    
    struct IntrinsicCamParams{
        double reprojectionError = -1;
        cv::Mat intrinsicMatrix;
        cv::Mat distortionCoef;
    };


    struct ColorThresholdParams{
        int hue[2] = {-1, -1};
        int separation[2] = {255, 0};
        int value[2] = {255, 0};
    };
    

    struct ExtrinsicPair
    {
        std::vector<std::pair<ImVec2, ImVec2>> points;
        ImVec2 zoom1_A;
        ImVec2 zoom1_B;

        ImVec2 zoom2_A;
        ImVec2 zoom2_B;
    };
}

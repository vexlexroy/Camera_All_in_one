#pragma once

#include "FrameBase.hpp"

#include <memory>
#include <vector>
#include <opencv2/core/matx.hpp>
#include <opencv2/core.hpp>


class FrameBase;

class FrameRelation {
public:
    std::shared_ptr<FrameBase> frame_src;
    std::shared_ptr<FrameBase> frame_destination;
    //Frame transformation?

    float distance_between_cams_in_cm;
    
    cv::Mat transformation_matrix;
    float transformation_matrix_reprojection_error;


    //TransformMatrix matrix1 =  transformTo(std::shared_ptr<FrameBase> goalFrame);
    //TransformMatrix matrix2 =  transformFrom(std::shared_ptr<FrameBase> goalFrame);

    //Add transform data if available...

private:

};

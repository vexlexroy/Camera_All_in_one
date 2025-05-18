#pragma once
#include "FrameBase.hpp"
#include "FrameRelation.hpp"
#include "GlobalParams.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <random>
#include <cmath>
#include <unordered_set>
#include <opencv2/core/matx.hpp>


//static std::mutex glMutex;
class Util{
    public:
        static std::string printHumanReadableFormat(long long int timeSinceEpochMicroseconds) {
            // Extract the seconds and remaining microseconds
            long long int totalSeconds = timeSinceEpochMicroseconds / 1000000;
            long long int microseconds = timeSinceEpochMicroseconds % 1000000;
            long long int milliseconds = microseconds / 1000;
            microseconds = microseconds % 1000;

            // Create a time_point representing the given duration since the epoch
            std::chrono::seconds durationSeconds(totalSeconds);
            std::chrono::time_point<std::chrono::system_clock> time_point(durationSeconds);

            // Convert to std::time_t to use with C standard library functions
            std::time_t time_t_value = std::chrono::system_clock::to_time_t(time_point);

            // Convert to local time in a thread-safe manner
            std::tm local_time;
            localtime_s(&local_time, &time_t_value);

            // Print the time in a human-readable format
            std::ostringstream oss;
            oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S")
                    << "." << std::setw(3) << std::setfill('0') << milliseconds
                    << "." << std::setw(3) << std::setfill('0') << microseconds;
            return oss.str();
        }
        static long long int timeSinceEpochMicroseconds(){
            auto now = std::chrono::system_clock::now();

            // Convert the current time point to microseconds since epoch
            auto microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(
                now.time_since_epoch()).count();

            return microseconds_since_epoch;
        }

        static int distLeft(int reference, int checkPoint, int bufferValue){
            if(checkPoint > reference)
            {
                reference += bufferValue;
            }

            return reference - checkPoint;
        }
        static int distRight(int reference, int checkPoint, int bufferValue){
            if(checkPoint < reference)
            {
                checkPoint += bufferValue;
            }
            
            return checkPoint - reference;
        }

        static bool isInRange(int value, int leftRange, int rightRange, int bufferValue){

            int range_abs = Util::distLeft(rightRange, leftRange, bufferValue);
            if((Util::distLeft(rightRange, value, bufferValue) <= range_abs)
            && (Util::distRight(leftRange, value, bufferValue) <= range_abs))
            {
                return true;
            }
            return false;
        } 

        static int getAvgValue(int leftRange, int rightRange, int bufferValue){

            int range_abs = Util::distLeft(rightRange, leftRange, bufferValue);
            int finalValue = rightRange - range_abs/2;
            if(finalValue < 0)
            {
                finalValue += bufferValue;
            }
            return finalValue;
        } 



        static void delay(int durationInMiliseconds){
            Sleep(durationInMiliseconds);
        }


        static void mat2Texture(const cv::Mat &mat, const cv::Mat &matFilter, GLuint& textureID) {

            


            if(textureID == 0){
                printf("Texture id = %d\n", textureID);
                glGenTextures(1, &textureID);
                printf("Texture id = %d\n", textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }



            
            if(mat.empty() && mat.type() != CV_8UC3){
                return;
            }

            //Filter must be grayscale...
            if(matFilter.empty() || matFilter.type() != CV_8UC1){
                return;
            }

            cv::Size size1 = mat.size();
            cv::Size size2 = matFilter.size();
            
            //Must be same size...
            if (size1.width != size2.width || size1.height != size2.height) {
                return;
            } 


            //
            cv::Mat matRGBA;
            cv::cvtColor(mat, matRGBA, cv::COLOR_BGR2RGBA);
            

            //Now change alpha based on filter...
            for(int x = 0; x < matRGBA.cols; x++){
                for(int y = 0; y < matRGBA.rows; y++){
                    matRGBA.at<cv::Vec4b>(y, x)[3] = matFilter.at<uchar>(y,x);      
                }
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, matRGBA.cols, matRGBA.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, matRGBA.data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        /*
        static void DrawDashedLine(ImDrawList* drawList, const ImVec2& imagePos, const ImVec2 finalPoints[2], float dashLength = 10.0f, float gapLength = 5.0f) {
            // Izračunavanje vektora koji predstavlja liniju
            ImVec2 lineDir = finalPoints[1] - finalPoints[0];
            float lineLength = ImLength(lineDir);
            lineDir = ImNormalize(lineDir);  // Normalizacija vektora

            float currentLength = 0.0f;
            bool drawDash = true;
            
            // Crtanje isprekidane linije
            while (currentLength < lineLength) {
                float dashEnd = currentLength + dashLength;
                if (dashEnd > lineLength) {
                    dashEnd = lineLength;
                }

                // Izračunavanje početne i završne tačke segmenta
                ImVec2 start = finalPoints[0] + lineDir * currentLength;
                ImVec2 end = finalPoints[0] + lineDir * dashEnd;

                // Crtanje segmenta
                if (drawDash) {
                    drawList->AddLine(ImVec2(imagePos.x + start.x, imagePos.y + start.y), ImVec2(imagePos.x + end.x, imagePos.y + end.y), IM_COL32(255, 255, 255, 255), 4.0f);
                }

                // Prelazak na sledeći segment
                currentLength += dashLength + gapLength;
                drawDash = !drawDash;  // Naizmenično crtanje i praznina
            }
        }
        */




        static void drawStar(cv::Mat* image, cv::Point center, int n, int radius_min, int radius_max, cv::Scalar color, int alpha) {
            double angle_step = CV_PI / n;
            std::vector<cv::Point> points;

            // Calculate vertices of the star
            for (int i = 0; i < n * 2; ++i) {
                int radius = (i % 2 == 0) ? radius_max : radius_min;
                double angle = i * angle_step;
                cv::Point pt(center.x + radius * std::cos(angle), center.y + radius * std::sin(angle));
                points.push_back(pt);
            }

            // Fill the interior of the star with partially transparent color
            std::vector<std::vector<cv::Point>> contours;
            contours.push_back(points);

            // Create a mask for the star area
            cv::Mat mask(image->rows, image->cols, CV_8UC1, cv::Scalar(0));
            cv::fillPoly(mask, contours, cv::Scalar(255)); // Set mask to white inside the star area
            
            

                
            // Blend the star area with the existing image
            cv::Mat blended;
            cv::addWeighted(*image, 1.0 - (alpha / 255.0), cv::Scalar(color[0], color[1], color[2]), alpha / 255.0, 0, blended);

            // Copy the blended star area to the image using the mask
            blended.copyTo(*image, mask);

            // Draw a cross in the middle of the star
            cv::line(*image, cv::Point(center.x, center.y - 10), cv::Point(center.x, center.y + 10), cv::Scalar(0, 0, 255), 2);
            cv::line(*image, cv::Point(center.x - 10, center.y), cv::Point(center.x + 10, center.y), cv::Scalar(0, 0, 255), 2);
            
            cv::polylines(*image, contours, true, cv::Scalar(0, 0, 255), 2);
        }

        static std::shared_ptr<cv::Mat> findTransformation(std::shared_ptr<FrameBase> frame_src, std::shared_ptr<FrameBase> frame_destination)
        {

            std::vector<std::shared_ptr<FrameRelation>> open_relations = std::vector<std::shared_ptr<FrameRelation>>(GlobalParams::getInstance().getCamRelations());
            if(frame_src->frameNickName == frame_destination->frameNickName)
            {
                return std::make_shared<cv::Mat>(cv::Mat::eye(4, 4, CV_64F));  // or CV_32F for float precision
            }
            std::vector<std::vector<std::shared_ptr<FrameRelation>>> possiblePaths;
            std::unordered_set<std::string> visited;

            //When i find best match i simply combine Frame relations of final

            //Eliminate all relations that do not have distance relation
            for(int i = 0; i < open_relations.size(); i++)
            {
                if(open_relations[i]->distance_between_cams_in_cm < 0)
                {
                    open_relations.erase(open_relations.begin() + i);
                    i--;
                }
            }

            int checkedNodes = 0;
            int removedFromOpenRelations = 0;
            //Put on possiblePaths that go from 
            for(int i = 0; i < open_relations.size(); i++)
            {
                if(open_relations[i]->frame_src->frameNickName == frame_src->frameNickName)
                {
                    possiblePaths.push_back(std::vector<std::shared_ptr<FrameRelation>>{open_relations[i]});

                }
                else if(open_relations[i]->frame_destination->frameNickName == frame_src->frameNickName)
                {
                    possiblePaths.push_back(std::vector<std::shared_ptr<FrameRelation>>{Util::reverseTransform(open_relations[i])});
                
                }
                else{
                    continue;
                }

                //Mark visited as true
                visited.insert(open_relations[i]->frame_src->frameNickName);
                visited.insert(open_relations[i]->frame_destination->frameNickName);
                //Delete element that was added
                open_relations.erase(open_relations.begin() + i);
                i--;
                removedFromOpenRelations++;
            }


            while(checkedNodes != possiblePaths.size())
            {
                std::vector<std::shared_ptr<FrameRelation>> expand = possiblePaths[checkedNodes++];

                std::shared_ptr<FrameRelation> elementThatShouldBeExpanded = expand.back();

                for(int i = 0; i < open_relations.size(); i++)
                {
                    if((elementThatShouldBeExpanded->frame_destination->frameNickName == open_relations[i]->frame_src->frameNickName)
                    && (visited.find(open_relations[i]->frame_destination->frameNickName) == visited.end()))
                    {
                        std::vector<std::shared_ptr<FrameRelation>> newRelationVector(expand);
                        newRelationVector.push_back(open_relations[i]);
                        possiblePaths.push_back(newRelationVector);

                    }
                    else if((elementThatShouldBeExpanded->frame_destination->frameNickName == open_relations[i]->frame_destination->frameNickName)
                    && (visited.find(open_relations[i]->frame_src->frameNickName) == visited.end()))
                    {
                        std::vector<std::shared_ptr<FrameRelation>> newRelationVector(expand);
                        newRelationVector.push_back(Util::reverseTransform(open_relations[i]));
                        possiblePaths.push_back(newRelationVector);

                    }
                    else{
                        continue;
                    }

                    //Mark visited as true
                    visited.insert(open_relations[i]->frame_src->frameNickName);
                    visited.insert(open_relations[i]->frame_destination->frameNickName);
                    //Delete element that was added
                    open_relations.erase(open_relations.begin() + i);
                    i--;
                }
            }


            //Now all paths should be calculated from src frame.
            //Now to get transformation i want i just need to see which path ends with my wanted frame_destination...
            std::shared_ptr<cv::Mat> finalTransformationMatrix = nullptr;
            for(int i = 0; i < possiblePaths.size(); i++)
            {
                if(possiblePaths[i].back()->frame_destination->frameNickName == frame_destination->frameNickName)
                {
                    finalTransformationMatrix = std::make_shared<cv::Mat>(Util::incorporateScale(possiblePaths[i].front()->transformation_matrix, possiblePaths[i].front()->distance_between_cams_in_cm));
                    //Thats it now calculate final transformation matrix and return it
                    for(int j = 1; j < possiblePaths[i].size(); j++)
                    {

                        *finalTransformationMatrix =  Util::incorporateScale(possiblePaths[i][j]->transformation_matrix, possiblePaths[i][j]->distance_between_cams_in_cm)* (*finalTransformationMatrix);
                    }

                    return finalTransformationMatrix;
                }
            }

            return finalTransformationMatrix;
        }
        

        
        static cv::Point3f transform3DPoint(cv::Point3f point3D, std::shared_ptr<FrameBase> frame_src, std::shared_ptr<FrameBase> frame_destination)
        {
            // Retrieve the 4x4 transformation matrix
            std::shared_ptr<cv::Mat> transformation_matrix = Util::findTransformation(frame_src, frame_destination);

            if (!transformation_matrix || transformation_matrix->empty() || transformation_matrix->rows != 4 || transformation_matrix->cols != 4)
            {
                // Return invalid point if the transformation matrix is invalid
                printf("Point is invalid\n");
                return cv::Point3f(-1, -1, -1);
            }
            //std::cout << "transformation_matrix: " << *transformation_matrix << std::endl;
            // Extract rotation matrix (3x3) and translation vector (1x3) from the transformation matrix
            cv::Mat rotation_matrix = transformation_matrix->rowRange(0, 3).colRange(0, 3);
            cv::Mat translation_vector = transformation_matrix->rowRange(0, 3).col(3);

            // Convert point3D to cv::Mat for matrix operations
            cv::Mat point3D_mat = (cv::Mat_<double>(4, 1) << point3D.x, point3D.y, point3D.z, 1);

            // Subtract translation vector
            cv::Mat point_in_target_frame = *transformation_matrix * point3D_mat;//(point3D_mat - translation_vector).t() * rotation_matrix;

            // Convert the result back to cv::Point3f
            cv::Point3f final_point(
                static_cast<float>(point_in_target_frame.at<double>(0, 0)) / point_in_target_frame.at<double>(0, 3),
                static_cast<float>(point_in_target_frame.at<double>(0, 1)) / point_in_target_frame.at<double>(0, 3),
                static_cast<float>(point_in_target_frame.at<double>(0, 2)) / point_in_target_frame.at<double>(0, 3)
            );

            return final_point;
        }

        
        static cv::Point2f point3D_to_camCordinates(cv::Point3f point3D, std::shared_ptr<FrameBase> point3DCordSystem, std::shared_ptr<FrameCam> targetCamFrame)
        {
            // Transform the 3D point into the target camera frame
            cv::Point3f point3DInTargetFrame = Util::transform3DPoint(point3D, point3DCordSystem, targetCamFrame);
            //std::cout << "point3DInTargetFrame" << point3DInTargetFrame << std::endl;

            // Camera intrinsic parameters
            cv::Mat cameraMatrix = targetCamFrame->intrinsicParams.intrinsicMatrix;
            cv::Mat distCoeffs = targetCamFrame->intrinsicParams.distortionCoef;

            // 3D point as a vector of points (required by cv::projectPoints)
            std::vector<cv::Point3f> objectPoints = { point3DInTargetFrame };
            std::vector<cv::Point2f> imagePoints;

            // Check if the point is behind the camera
            if (point3DInTargetFrame.z <= 0)
            {
                // If the point is behind the camera, return an invalid coordinate
                return cv::Point2f(-1, -1);
            }

            // Project the 3D point to the image plane
            cv::projectPoints(objectPoints, cv::Vec3d(0, 0, 0), cv::Vec3d(0, 0, 0), cameraMatrix, distCoeffs, imagePoints);

            // Extract the projected point
            cv::Point2f uv = imagePoints[0];

            // Get the frame dimensions
            int frameWidth = 640;
            int frameHeight = 480;

            // Clamp the point to the valid image boundaries
            if (uv.x < 0)
                uv.x = 0;
            if (uv.y < 0)
                uv.y = 0;
            if (uv.x >= frameWidth)
                uv.x = frameWidth - 1;
            if (uv.y >= frameHeight)
                uv.y = frameHeight - 1;

            return uv;
        }
        static cv::Mat incorporateScale(cv::Mat transformationMatrix, float distance_in_cm)
        {
            // Assuming transformationMatrix is a 4x4 matrix where the top-left 3x3 part is the rotation matrix
            // and the top-right 3x1 part is the translation vector.
            

            // Get the translation vector (the last column except the last row)
            cv::Mat translation = transformationMatrix(cv::Rect(3, 0, 1, 3)).clone(); // This extracts the translation vector
            
            // Scale the translation vector by the distance
            translation *= distance_in_cm;
            
            //std::cout << "translation" << translation << std::endl;
            // Replace the translation part in the transformation matrix with the scaled translation


            cv::Mat new_translation(transformationMatrix.clone());

            translation.copyTo(new_translation(cv::Rect(3, 0, 1, 3)));
            
            return new_translation;

        }
        static std::shared_ptr<cv::Point3f> triangulate(std::shared_ptr<FrameCam> cam1, cv::Point2f cam1_2D_point,
                                                std::shared_ptr<FrameCam> cam2, cv::Point2f cam2_2D_point,
                                                std::shared_ptr<FrameBase> targetFrame)
        {
            std::shared_ptr<cv::Point3f> result = nullptr;
            
            //printf("cam1_2D_point: (%f, %f)\n", cam1_2D_point.x, cam1_2D_point.y);
            //printf("cam2_2D_point: (%f, %f)\n", cam2_2D_point.x, cam2_2D_point.y);

            // 1st check: Transformation from cam1 to cam2 must exist
            std::shared_ptr<cv::Mat> cam1_cam2_transformation = Util::findTransformation(cam1, cam2);
            if (cam1_cam2_transformation == nullptr)
            {
                printf("cam1_cam2_transformation == nullptr\n");
                return result;
            }

            // 2nd check: Transformation from cam1 to targetFrame must exist
            std::shared_ptr<cv::Mat> cam1_targetFrame_transformation = Util::findTransformation(cam1, targetFrame);
            if (cam1_targetFrame_transformation == nullptr)
            {
                printf("cam1_targetFrame_transformation == nullptr\n");
                return result;
            }
            //printf("Can be calculated\n");
            
            // Camera parameters
            cv::Mat cameraMatrix1 = cam1->intrinsicParams.intrinsicMatrix;
            cv::Mat distCoeffs1 = cam1->intrinsicParams.distortionCoef;
            cv::Mat cameraMatrix2 = cam2->intrinsicParams.intrinsicMatrix;
            cv::Mat distCoeffs2 = cam2->intrinsicParams.distortionCoef;

            //std::cout << "cameraMatrix1:" << cameraMatrix1 << std::endl;
            //std::cout << "distCoeffs1:" << distCoeffs1 << std::endl;
            //std::cout << "cameraMatrix2:" << cameraMatrix2 << std::endl;
            //std::cout << "distCoeffs2:" << distCoeffs2 << std::endl;

            

            //std::cout << "firstMiddle:" << firstMiddle.x << ", " << firstMiddle.y << std::endl;
            //std::cout << "secondMiddle:" << secondMiddle.x << ", " << secondMiddle.y << std::endl;

            // Convert ImVec2 to cv::Point2f
            std::vector<cv::Point2f> points1 = { cam1_2D_point};
            std::vector<cv::Point2f> points2 = { cam2_2D_point };

            // Undistort points
            std::vector<cv::Point2f> undistortedPoints1, undistortedPoints2;
            cv::undistortPoints(points1, undistortedPoints1, cameraMatrix1, distCoeffs1);
            cv::undistortPoints(points2, undistortedPoints2, cameraMatrix2, distCoeffs2);

            if (undistortedPoints1.empty() || undistortedPoints2.empty()) {
                std::cerr << "Error: Undistorted points are empty." << std::endl;
                return result;
            }
            // Projection matrices (P1 and P2)
            // Assuming P1 is the identity matrix and P2 is based on the relative pose (R, t) between cameras
            cv::Mat R, t;
            // Obtain R and t from the camera extrinsic parameters
            cv::Mat P1 = cv::Mat::eye(3, 4, CV_64F); // Camera 1 projection matrix (identity)
            cv::Mat P2(3, 4, CV_64F);               // Camera 2 projection matrix

            
            cv::Mat rotationMatrix = (*cam1_cam2_transformation)(cv::Rect(0, 0, 3, 3));
            rotationMatrix.copyTo(P2(cv::Rect(0, 0, 3, 3))); // Copy the rotation into P2

            // Extract the 3x1 translation vector from cam1_cam2_transformation (top-right 3x1 part)
            cv::Mat translationMatrix = (*cam1_cam2_transformation)(cv::Rect(3, 0, 1, 3));
            translationMatrix.copyTo(P2(cv::Rect(3, 0, 1, 3))); // Copy the translation into P2


            // Triangulate points
            cv::Mat points4D;
            cv::triangulatePoints(P1, P2, undistortedPoints1, undistortedPoints2, points4D);
            
            //scale point4d to vector that is 

            //std::cout << points4D << std::endl;

            cv::Mat firstPoint = points4D.col(0);  // Get the first column

            // Convert from homogeneous to Cartesian coordinates
            float w = firstPoint.at<float>(3);  // Extract the homogeneous coordinate (w)
            cv::Point3f point3D(firstPoint.at<float>(0) / w,  // X coordinate
                                firstPoint.at<float>(1) / w,  // Y coordinate
                                firstPoint.at<float>(2) / w); // Z coordinate

            

            cv::Mat onesVector = cv::Mat::ones(4, 1, CV_64F);
            onesVector.at<double>(0, 0) = static_cast<double>(point3D.x);  // X coordinate
            onesVector.at<double>(1, 0) = static_cast<double>(point3D.y);  // Y coordinate
            onesVector.at<double>(2, 0) = static_cast<double>(point3D.z);  // Z coordinate
            onesVector.at<double>(3, 0) = 1.0;  // Homogeneous coordinate (w)


            cv::Mat finaly = (*cam1_targetFrame_transformation) *onesVector;

            double x_h = finaly.at<double>(0, 0);  // Transformed x
            double y_h = finaly.at<double>(1, 0);  // Transformed y
            double z_h = finaly.at<double>(2, 0);  // Transformed z
            double w_h = finaly.at<double>(3, 0);  // Homogeneous coordinate (w)

            // Convert to Cartesian coordinates by dividing by w
            cv::Point3f transformedPoint(static_cast<float>(x_h / w_h),  // X coordinate
                              static_cast<float>(y_h / w_h),  // Y coordinate
                              static_cast<float>(z_h / w_h)); // Z coordinate


            //std::cout << "cam1_targetFrame_transformation" << *cam1_targetFrame_transformation << std::endl;
            //std::cout << "vec*cam1_targetFrame_transformation" <<  << std::endl;
            // Create a shared pointer to tht
            result = std::make_shared<cv::Point3f>(transformedPoint);

            
            //std::cout << "result= "<< multiplied << std::endl;
            return result;
           
            
        }

        static std::shared_ptr<cv::Mat> getFundamentalMatrix(std::shared_ptr<FrameCam> cam1 , std::shared_ptr<FrameCam> cam2)
        {
            cv::Mat cameraMatrix1 = cam1->intrinsicParams.intrinsicMatrix;
            cv::Mat distCoeffs1 = cam1->intrinsicParams.distortionCoef;
            cv::Mat cameraMatrix2 = cam2->intrinsicParams.intrinsicMatrix;
            cv::Mat distCoeffs2 = cam2->intrinsicParams.distortionCoef;


            std::shared_ptr<cv::Mat> cam1_cam2_transformation = Util::findTransformation(cam1, cam2); // it is 4*4 and has rotation and translation inside

            if(cam1_cam2_transformation == nullptr)
            {
                return nullptr;
            }

            // Extract rotation and translation
            cv::Mat transformation = *cam1_cam2_transformation;
            cv::Mat rotation = transformation(cv::Range(0, 3), cv::Range(0, 3)); // Top-left 3x3 submatrix
            cv::Mat translation = transformation(cv::Range(0, 3), cv::Range(3, 4)); // Top-right 3x1 column

            // Compute the skew-symmetric matrix of the translation vector
            cv::Mat tx = (cv::Mat_<double>(3, 3) <<
                0, -translation.at<double>(2, 0), translation.at<double>(1, 0),
                translation.at<double>(2, 0), 0, -translation.at<double>(0, 0),
                -translation.at<double>(1, 0), translation.at<double>(0, 0), 0);

            // Compute the essential matrix: E = [t]_x * R
            cv::Mat essentialMatrix = tx * rotation;

            // Compute the fundamental matrix: F = (K2^-T) * E * (K1^-1)
            cv::Mat fundamentalMatrix = cameraMatrix2.inv().t() * essentialMatrix * cameraMatrix1.inv();

            // Wrap the result in a shared_ptr
            return std::make_shared<cv::Mat>(fundamentalMatrix);
        }

        static std::shared_ptr<cv::Point3f> getEpipolarLineCoefficiens(std::shared_ptr<FrameCam> cam1 , cv::Point2f cam1Point, std::shared_ptr<FrameCam> cam2)
        {
            std::shared_ptr<cv::Mat> fundamental_matrix = Util::getFundamentalMatrix(cam1, cam2);
            if(fundamental_matrix == nullptr)
            {
                return nullptr;
            }


            //finish function that it returns [a, b, c] coeficiens on second image. Line ax + by = c

            // Convert the 2D point from the first camera to homogeneous coordinates
            cv::Mat point1 = (cv::Mat_<double>(3, 1) << cam1Point.x, cam1Point.y, 1.0);

            // Calculate the epipolar line in the second image: l2 = F * x1
            cv::Mat epipolarLine = *fundamental_matrix * point1;

            // Convert the result to a cv::Point3f (a, b, c) coefficients
            float a = epipolarLine.at<double>(0, 0);
            float b = epipolarLine.at<double>(1, 0);
            float c = epipolarLine.at<double>(2, 0);

            // Return the epipolar line coefficients as a shared_ptr
            return std::make_shared<cv::Point3f>(a, b, c);
        }

        static cv::Point3f findClosestPointToPlane(float plane_normal_x, float plane_normal_y, float plane_normal_z,
                                                    float plane_point_x, float plane_point_y, float plane_point_z,
                                                    float point_x, float point_y, float point_z)
        {
            float t = (plane_normal_x * (plane_point_x - point_x) +
               plane_normal_y * (plane_point_y - point_y) +
               plane_normal_z * (plane_point_z - point_z)) /
              (plane_normal_x * plane_normal_x +
               plane_normal_y * plane_normal_y +
               plane_normal_z * plane_normal_z);
            return cv::Point3f(point_x + t * plane_normal_x,
                       point_y + t * plane_normal_y,
                       point_z + t * plane_normal_z);
        }


        static std::vector<int> retrieveFreeCams()
        {
            std::vector<int> freeCams;
            std::unordered_set<int> ocupiedCams = GlobalParams::getInstance().openedDShowCameras;


            for (int i = 0; i < 10; ++i) {  // Adjust the range if needed
                if(ocupiedCams.find(i) != ocupiedCams.end())
                {
                    continue;
                }
                cv::VideoCapture cap(i, cv::CAP_DSHOW);  // Open camera using DShow backend
                
                if (cap.isOpened()) {
                    freeCams.push_back(i);
                    cap.release();  // Close the camera after checking
                }
            }
            return freeCams;
        }

        
        // Function to compute the intersection of a line with a plane
        static cv::Point3f line_plane_intersection(const cv::Point3f& line_point, const cv::Point3f& line_dir, 
                                            const cv::Point3f& plane_normal, const cv::Point3f& plane_point) {
            // Compute the dot product between line direction and plane normal
            float dot = line_dir.dot(plane_normal);
            if (std::abs(dot) < 1e-6) {
                // If the line is parallel to the plane, no intersection
                std::cerr << "Line and plane are parallel, no intersection" << std::endl;
                return cv::Point3f(0, 0, 0); // Return an invalid point
            }

            // Compute the vector from the line point to the plane point
            cv::Point3f line_to_plane = plane_point - line_point;

            // Calculate the intersection scalar t
            float t = line_to_plane.dot(plane_normal) / dot;

            // Compute the intersection point
            cv::Point3f intersection = line_point + t * line_dir;
            return intersection;
        }

        static cv::Mat normalizeVector3D(const cv::Point3f& point) {
            float magnitude = std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
            if (magnitude == 0.0f) {
                throw std::invalid_argument("Cannot normalize a zero-length vector.");
            }

            // Create a 3x1 cv::Mat and initialize it with the normalized values
            cv::Mat normalizedMat = (cv::Mat_<float>(3, 1) << point.x / magnitude, point.y / magnitude, point.z / magnitude);
            
            return normalizedMat;
        }


        static cv::Point2f closestPointOnLine(float a, float b, float c, cv::Point2f point) {
            
            // Calculate the distance from the point to the line
            float denominator = a * a + b * b;
            
            float x2 = (b * (b * point.x - a * point.y) - a * c) / denominator;
            float y2 = (a * (-b * point.x + a * point.y) - b * c) / denominator;
            
            return cv::Point2f{x2, y2};
        }

        static std::shared_ptr<FrameRelation> reverseTransform(std::shared_ptr<FrameRelation> original) {
            std::shared_ptr<FrameRelation> reversedTransform = std::make_shared<FrameRelation>();
            reversedTransform->frame_src = original->frame_destination;
            reversedTransform->frame_destination = original->frame_src;
            reversedTransform->distance_between_cams_in_cm = original->distance_between_cams_in_cm;
            cv::Mat T_inverse;
            cv::invert(original->transformation_matrix, T_inverse);
            reversedTransform->transformation_matrix = T_inverse;
            reversedTransform->transformation_matrix_reprojection_error = original->transformation_matrix_reprojection_error;
            return reversedTransform;
        }
        static void mat2Texture(const cv::Mat &mat, GLuint& textureID) {

            


            if(textureID == 0){
                printf("Texture id = %d\n", textureID);
                glGenTextures(1, &textureID);
                printf("Texture id = %d\n", textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if(mat.empty()){
                return;
            }
            cv::Mat matRGBA;
            if (mat.channels() == 3) {
                cv::cvtColor(mat, matRGBA, cv::COLOR_BGR2RGBA);
            } else if (mat.channels() == 1) {
                cv::cvtColor(mat, matRGBA, cv::COLOR_GRAY2RGBA);
            } else if (mat.channels() == 4) {
                matRGBA = mat;
            }
            

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, matRGBA.cols, matRGBA.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, matRGBA.data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        static void drawCenter(cv::Mat& image, cv::Point center, int size, const ImVec4& color, int thickness) {
            // Convert ImVec4 color to cv::Scalar
            cv::Scalar cvColor = Util::ImVec4ToScalar(color);

            // Calculate the endpoints of the X
            cv::Point pt1(center.x - size / 2, center.y - size / 2);
            cv::Point pt2(center.x + size / 2, center.y + size / 2);
            cv::Point pt3(center.x - size / 2, center.y + size / 2);
            cv::Point pt4(center.x + size / 2, center.y - size / 2);

            //printf("P1 = (%d, %d), P2 = (%d, %d)\n", pt1.x, pt1.y, pt2.x, pt2.y);
            // Draw the two lines of the X
            cv::line(image, pt1, pt2, cvColor, thickness);
            cv::line(image, pt3, pt4, cvColor, thickness);

        }
        static void drawText(cv::InputOutputArray img, const cv::String &text, cv::Point org, double fontScale, const ImVec4& color, int thickness = 1){
            int font = cv::FONT_HERSHEY_SIMPLEX;
            cv::Scalar cvColor = Util::ImVec4ToScalar(color);

            cv::putText(img, text, org, font, fontScale, cvColor, 0.3);
            org.x += 1;
            cv::putText(img, text, org, font, fontScale, cvColor, 0.3);
        }

        static cv::Scalar ImVec4ToScalar(const ImVec4& color) {
            return cv::Scalar(color.z * 255, color.y * 255, color.x * 255); // BGR format
        }
        static bool isSingleInteger(const char* str) {
            // Check if the string is empty
            if (str == nullptr || *str == '\0') {
                return false;
            }

            // Check for a sign character at the beginning
            if (*str == '-' || *str == '+') {
                str++;
            }

            // Check if all characters are digits
            while (*str) {
                if (!std::isdigit(*str)) {
                    return false;
                }
                str++;
            }

            return true;
        }

        static std::vector<std::vector<double>> matToVector(const cv::Mat& mat){
            std::vector<std::vector<double>> vec(mat.rows, std::vector<double>(mat.cols));

            for (int i = 0; i < mat.rows; ++i) {
                for (int j = 0; j < mat.cols; ++j) {
                    vec[i][j] = mat.at<double>(i, j);
                }
            }
            return vec; 
        }


        static cv::Mat vectorToMat(const std::vector<std::vector<double>> vector){

            if (vector.empty() || vector[0].empty()) {
                return cv::Mat(); // Return an empty Mat if the input vector is empty
            }

            int rows = vector.size();
            int cols = vector[0].size();

            cv::Mat mat(rows, cols, CV_64F); // Create a Mat of the same size with type double

            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    mat.at<double>(i, j) = vector[i][j];
                }
            }

            return mat;
            
        }

        static ImVec4 getImVec4FromString(const std::string& nickname) {
            // Convert string to seed
            std::hash<std::string> hasher;
            unsigned int seed = static_cast<unsigned int>(hasher(nickname));

            // Initialize PRNG with seed
            std::mt19937 rng(seed);

            // Define distribution for RGB components
            std::uniform_real_distribution<float> dist(0.3f, 1.0f);

            // Generate random floats for RGB components
            for(int i = 0; i < 10; i++){ // make it even more random...
                dist(rng);
            }
            
            float h = dist(rng);
            float s = 1.0f;
            float v = 1.0f;
            //printf("%.2f, %.2f, %.2f\n", r, g, b);
            cv::Mat rgb; 
            cv::cvtColor(cv::Mat(1, 1, CV_32FC3, cv::Scalar(h*360, s, v)), rgb, cv::COLOR_HSV2RGB); 
            cv::Vec3f rgbVec = rgb.at<cv::Vec3f>(0, 0); 
            return ImVec4(rgbVec[0], rgbVec[1], rgbVec[2], 1.0f);
            //return ImVec4(r, g, b, 1.0f); // Alpha set to 1.0 (fully opaque)
        }
        static ImVec4 HexToImVec4(const std::string& hex) {
            // Ensure the string is in the correct format
            if (hex.length() != 9 || hex[0] != '#') {
                return ImVec4(0, 0, 0, 1); // Return black color with full opacity if format is incorrect
            }

            unsigned int r, g, b, a;
            std::stringstream ss;
            ss << std::hex << hex.substr(1);
            ss >> std::setw(2) >> r >> std::setw(2) >> g >> std::setw(2) >> b >> std::setw(2) >> a;

            return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        }


        static double computeReprojectionError(
    const std::vector<cv::Point2f>& undistortedPoints1,
    const std::vector<cv::Point2f>& undistortedPoints2,
    const cv::Mat& R,
    const cv::Mat& T,
    const cv::Mat& cameraMatrix1,
    const cv::Mat& cameraMatrix2
    ) {
            std::vector<cv::Point3f> points1_3D;
            for (const auto& p : undistortedPoints1) {
                points1_3D.emplace_back(cv::Point3f(p.x, p.y, 1.0f));
            }

            // Transform points using R and T
            std::vector<cv::Point3f> points1_transformed;
            for (const auto& p : points1_3D) {
                cv::Mat ptMat = (cv::Mat_<double>(3, 1) << p.x, p.y, p.z);
                cv::Mat transformedPt = R * ptMat + T;
                points1_transformed.emplace_back(cv::Point3f(transformedPt.at<double>(0), transformedPt.at<double>(1), transformedPt.at<double>(2)));
            }

            // Project transformed points to the second image plane
            std::vector<cv::Point2f> projectedPoints;
            for (const auto& p : points1_transformed) {
                cv::Mat ptMat = (cv::Mat_<double>(3, 1) << p.x, p.y, p.z);
                cv::Mat projectedPt = cameraMatrix2 * ptMat;
                projectedPoints.emplace_back(cv::Point2f(projectedPt.at<double>(0) / projectedPt.at<double>(2), projectedPt.at<double>(1) / projectedPt.at<double>(2)));
            }

            // Compute reprojection error
            double totalError = 0.0;
            for (size_t i = 0; i < undistortedPoints2.size(); ++i) {
                double error = norm(undistortedPoints2[i] - projectedPoints[i]);
                totalError += error;
            }

            return totalError / undistortedPoints2.size();
        }
};


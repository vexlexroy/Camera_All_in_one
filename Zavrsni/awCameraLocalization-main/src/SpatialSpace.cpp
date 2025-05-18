#include "SpatialState.hpp"

#include "FrameBase.hpp"
#include "FrameCam.hpp"
#include "FrameCustom.hpp"
#include "FrameRelation.hpp"
#include "Util.hpp"

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

void SpatialState::loadFromJson(nlohmann::json& jsonData){
    
    //ako ne postoji traženi zapisi
    if(!jsonData.contains("spatialSpace")){
        return;
    }


    //Extracting camFrames
    if(jsonData["spatialSpace"].contains("camFrames")){
        auto& camFrames = jsonData["spatialSpace"]["camFrames"];
        for (auto camFrame = camFrames.begin(); camFrame != camFrames.end(); ++camFrame) {
            printf("Postoji 1 cam frame, %s\n", camFrame.key().c_str());
            //camFrame.value()["frameNickName"]
            //camFrame.value()["frameUrlName"
            
            Structs::IntrinsicCamParams intrinsicParams;
            if(camFrame.value().contains("intrinsicParams")){
                intrinsicParams.reprojectionError = camFrame.value()["intrinsicParams"]["reprojectionError"].get<float>();
                
                std::vector<std::vector<double>> intrinsicMatrixVector = camFrame.value()["intrinsicParams"]["intrinsicMatrix"].get<std::vector<std::vector<double>>>();
                intrinsicParams.intrinsicMatrix = Util::vectorToMat(intrinsicMatrixVector);

                std::vector<std::vector<double>> distortionCoefsVector = camFrame.value()["intrinsicParams"]["distortionCoef"].get<std::vector<std::vector<double>>>();
                intrinsicParams.distortionCoef = Util::vectorToMat(distortionCoefsVector);
            }
            Structs::ColorThresholdParams colorThresholdParams;
            colorThresholdParams.hue[0] = camFrame.value()["colorThresholdParams"]["hue"][0].get<int>();
            colorThresholdParams.hue[1] = camFrame.value()["colorThresholdParams"]["hue"][1].get<int>();
            colorThresholdParams.separation[0] = camFrame.value()["colorThresholdParams"]["separation"][0].get<int>();
            colorThresholdParams.separation[1] = camFrame.value()["colorThresholdParams"]["separation"][1].get<int>();
            colorThresholdParams.value[0] = camFrame.value()["colorThresholdParams"]["value"][0].get<int>();
            colorThresholdParams.value[1] = camFrame.value()["colorThresholdParams"]["value"][1].get<int>();


            std::shared_ptr<FrameCam> loadedFrame = std::make_shared<FrameCam>(camFrame.value()["frameUrlName"], camFrame.value()["frameNickName"], camFrame.value()["streamLagInMilisecond"].get<int>(), intrinsicParams, colorThresholdParams);
            
            this->camFrames.push_back(loadedFrame);
     
        }
    }

    if(jsonData["spatialSpace"].contains("customFrames")){
        auto& customFrames = jsonData["spatialSpace"]["customFrames"];
        for (auto customFrame = customFrames.begin(); customFrame != customFrames.end(); ++customFrame) {


            std::shared_ptr<FrameCustom> loadedFrame = std::make_shared<FrameCustom>(customFrame.value()["frameNickName"], customFrame.value()["cam1Nickname"], customFrame.value()["cam2Nickname"]);

            this->customFrames.push_back(loadedFrame);
            //json["spatialSpace"]["customFrames"][std::to_string(i)]["frameNickName"] = frameCustom->frameNickName;
            //json["spatialSpace"]["customFrames"][std::to_string(i)]["cam1Nickname"] = frameCustom->cam1Nickname;
            //json["spatialSpace"]["customFrames"][std::to_string(i)]["cam2Nickname"] = frameCustom->cam2Nickname;
        }
    }
    

    //Extracting frameWorld
    if(jsonData["spatialSpace"].contains("frameWorld")){
        printf("Postoji 1 world frame\n");
        

        //Find corresponding
        for(int i = 0; i < this->camFrames.size(); i++)
        {
            if(this->camFrames[i]->frameNickName == jsonData["spatialSpace"]["frameWorld"])
            {
                this->frameWorld = this->camFrames[i]->frameNickName;
            }
        }

        for(int i = 0; i < this->customFrames.size(); i++)
        {
            if(this->customFrames[i]->frameNickName == jsonData["spatialSpace"]["frameWorld"])
            {
                this->frameWorld = this->customFrames[i]->frameNickName;
            }
        }
    }
    


    //Extracting frameRelations...
    if(jsonData["spatialSpace"].contains("frameRelations")){
        auto& relations = jsonData["spatialSpace"]["frameRelations"];
        for (auto relation = relations.begin(); relation != relations.end(); ++relation) {

            std::string src_nickname = relation.value()["frame_src"];
            std::string dst_nickname = relation.value()["frame_destination"];

            std::shared_ptr<FrameBase> srcFrameCam = nullptr;
            std::shared_ptr<FrameBase> dstFrameCam = nullptr;

            //get coresponding CamFrame
            for(int i = 0; i < this->camFrames.size(); i++)
            {
                if((this->camFrames[i]->frameNickName == src_nickname))
                {
                    srcFrameCam = this->camFrames[i];
                }
                if((this->camFrames[i]->frameNickName == dst_nickname))
                {
                    dstFrameCam = this->camFrames[i];
                }
            }

            //get coresponding CamFrame
            for(int i = 0; i < this->customFrames.size(); i++)
            {
                if((this->customFrames[i]->frameNickName == dst_nickname))
                {
                    dstFrameCam = this->customFrames[i];
                }
            }

            if((srcFrameCam != nullptr) && (dstFrameCam != nullptr))
            {
                std::shared_ptr<FrameRelation> loadedRelation = std::make_shared<FrameRelation>();

                loadedRelation->frame_src = srcFrameCam;
                loadedRelation->frame_destination = dstFrameCam;
                loadedRelation->distance_between_cams_in_cm = relation.value()["distance_between_cams_in_cm"].get<float>();
                loadedRelation->transformation_matrix_reprojection_error = relation.value()["transformation_matrix_reprojection_error"].get<float>();
                
                std::vector<std::vector<double>> transformationMatrixVector = relation.value()["transformation_matrix"].get<std::vector<std::vector<double>>>();
                loadedRelation->transformation_matrix = Util::vectorToMat(transformationMatrixVector);

                //std::cout << "Transformation matrix = \n" << loadedRelation->transformation_matrix << std::endl;
                this->frameRelations.push_back(loadedRelation);
            }
            printf("Frame src = %s, frame destination = %s\n", src_nickname.c_str(), dst_nickname.c_str());
        }
    }
    
}

void SpatialState::saveToJson(nlohmann::json& json){
    printf("Saving to json\n");


    //Saving camFrames
    for(int i = 0; i < this->camFrames.size(); i++){
        std::shared_ptr<FrameCam> frameCam = this->camFrames[i];
        json["spatialSpace"]["camFrames"][std::to_string(i)]["frameNickName"] = frameCam->frameNickName;
        json["spatialSpace"]["camFrames"][std::to_string(i)]["frameUrlName"] = frameCam->frameUrlName;
        json["spatialSpace"]["camFrames"][std::to_string(i)]["streamLagInMilisecond"] = frameCam->streamLagInMilisecond;

        json["spatialSpace"]["camFrames"][std::to_string(i)]["colorThresholdParams"]["hue"] = frameCam->colorThresholdParams.hue;
        json["spatialSpace"]["camFrames"][std::to_string(i)]["colorThresholdParams"]["separation"] = frameCam->colorThresholdParams.separation;
        json["spatialSpace"]["camFrames"][std::to_string(i)]["colorThresholdParams"]["value"] = frameCam->colorThresholdParams.value;

        if(frameCam->intrinsicParams.reprojectionError >= 0){
            json["spatialSpace"]["camFrames"][std::to_string(i)]["intrinsicParams"]["reprojectionError"] = frameCam->intrinsicParams.reprojectionError;
        }
        
        
        std::vector<std::vector<double>> intrinsicMatrixVec = Util::matToVector(frameCam->intrinsicParams.intrinsicMatrix);
        if(intrinsicMatrixVec.size() == 3){
            json["spatialSpace"]["camFrames"][std::to_string(i)]["intrinsicParams"]["intrinsicMatrix"] = intrinsicMatrixVec;
        }
        

        std::vector<std::vector<double>> distortionCoefVec = Util::matToVector(frameCam->intrinsicParams.distortionCoef);
        printf("distortionCoefVec size = %d, %d, %d\n", distortionCoefVec.size(), frameCam->intrinsicParams.distortionCoef.size().height, frameCam->intrinsicParams.distortionCoef.size().width);
        if(distortionCoefVec.size() == 1){
            json["spatialSpace"]["camFrames"][std::to_string(i)]["intrinsicParams"]["distortionCoef"] = distortionCoefVec;
        }
        //json["spatialSpace"]["camFrames"][std::to_string(i)]["intrinsicParams"]["intrinsicMatrix"] = cv::matToVector(frameCam->intrinsicParams.intrinsicMatrix);
        
        //json["spatialSpace"]["camFrames"][std::to_string(i)]["intrinsicParams"]["distortionCoef"] = frameCam->intrinsicParams.distortionCoef;
        //printf("streamLagInMilisecond = %d\n", frameCam->streamLagInMilisecond);
    }

    //Saving frame relations
    for(int i = 0; i < this->frameRelations.size(); i++)
    {
        std::shared_ptr<FrameRelation> frameRelation = this->frameRelations[i];
        json["spatialSpace"]["frameRelations"][std::to_string(i)]["frame_src"] = frameRelation->frame_src->frameNickName;
        json["spatialSpace"]["frameRelations"][std::to_string(i)]["frame_destination"] = frameRelation->frame_destination->frameNickName;
        json["spatialSpace"]["frameRelations"][std::to_string(i)]["distance_between_cams_in_cm"] = frameRelation->distance_between_cams_in_cm;
        std::vector<std::vector<double>> transformationMatrixVector = Util::matToVector(frameRelation->transformation_matrix);
        if(transformationMatrixVector.size() == 4){
            json["spatialSpace"]["frameRelations"][std::to_string(i)]["transformation_matrix"] = transformationMatrixVector;
        }
        json["spatialSpace"]["frameRelations"][std::to_string(i)]["transformation_matrix_reprojection_error"] = frameRelation->transformation_matrix_reprojection_error;
    }

    //Saving custom frames
    for(int i = 0; i < this->customFrames.size(); i++){
        std::shared_ptr<FrameCustom> frameCustom = this->customFrames[i];

        json["spatialSpace"]["customFrames"][std::to_string(i)]["frameNickName"] = frameCustom->frameNickName;
        json["spatialSpace"]["customFrames"][std::to_string(i)]["cam1Nickname"] = frameCustom->cam1Nickname;
        json["spatialSpace"]["customFrames"][std::to_string(i)]["cam2Nickname"] = frameCustom->cam2Nickname;
    }

    //Saving frameWorld
    if(!this->frameWorld.empty()){
        json["spatialSpace"]["frameWorld"] = this->frameWorld;
    }
}


std::shared_ptr<FrameCam> SpatialState::openStream(const std::string& streamUrlName){
    for(int i = 0; i < this->camFrames.size(); i++){
        if(this->camFrames[i]->frameUrlName == streamUrlName){
            return this->camFrames[i];
        }
    }
    return nullptr;
}
void SpatialState::createStream(const std::string& streamUrlName, const std::string& nickname){
    Structs::IntrinsicCamParams defaultCamParams;
    Structs::ColorThresholdParams colorThresholdParams;
    std::shared_ptr<FrameCam> newFrameCame = std::make_unique<FrameCam>(streamUrlName, nickname, 0, defaultCamParams, colorThresholdParams);
    this->camFrames.push_back(newFrameCame);
}

void SpatialState::createCustomFrame(const std::string& nickName, const std::string& cam1Nickname, const std::string& cam2Nickname){

    std::shared_ptr<FrameCustom> newCustomFrame = std::make_unique<FrameCustom>(nickName, cam1Nickname, cam2Nickname);
    this->customFrames.push_back(newCustomFrame);
}


bool SpatialState::doesStreamExist(const std::string& streamUrlName){
    printf("CamFrames num = %d\n", this->camFrames.size());
    for(int i = 0 ; i < this->camFrames.size(); i++){
        if(this->camFrames[i]->frameUrlName == streamUrlName){
            return true;
        }
    }
    return false;
}
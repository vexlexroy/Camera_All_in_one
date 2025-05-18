#include "GlobalParams.hpp"
#include "Structs.hpp"
#include "SpatialState.hpp"
#include "FrameCam.hpp"

#include <stdio.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

GlobalParams& GlobalParams::getInstance() {
    static GlobalParams globalParamsInstance("globalParams.json");
    return globalParamsInstance;
}
void GlobalParams::saveParams(){
    //Convert data to nlohmann/json and then save it to file...
    nlohmann::json jsonParams;

    //Filling zoom
    jsonParams["zoom"]["scaleFactor"] = this->zoom.scaleFactor;
    jsonParams["zoom"]["scaleFactorMax"] = this->zoom.scaleFactorMax;
    jsonParams["zoom"]["scaleFactorMin"] = this->zoom.scaleFactorMin;
    jsonParams["zoom"]["scaleFactorSpeed"] = this->zoom.scaleFactorSpeed;

    //Filling isShowWork
    jsonParams["view"]["isShowWork"] = this->view.isShowWork;


    this->spatialState.saveToJson(jsonParams);


    std::ofstream file(this->pathToJson);
    if (file.is_open()) {
        file << jsonParams.dump(4); // dump(4) for pretty-printing with 4 spaces
        file.close();
        printf("GlobalParams saved to file: %s\n", this->pathToJson.c_str());
    }
    else {
        printf("GlobalParams error opening file: %s\n", this->pathToJson.c_str());
    }
}
GlobalParams::GlobalParams(const std::string& pathToJson){
    
    this->pathToJson = pathToJson;
    
    std::ifstream file(pathToJson);
    nlohmann::json jsonParams;

    Structs::ZoomParams zoomParams;
    Structs::ViewParams viewParams;
    if (file.is_open()) {
        file >> jsonParams;
        
        //Extracting ZoomParams
        zoomParams.scaleFactor = jsonParams["zoom"]["scaleFactor"].get<float>();
        zoomParams.scaleFactorMax = jsonParams["zoom"]["scaleFactorMax"].get<float>();
        zoomParams.scaleFactorMin = jsonParams["zoom"]["scaleFactorMin"].get<float>();
        zoomParams.scaleFactorSpeed = jsonParams["zoom"]["scaleFactorSpeed"].get<float>();
        

        //Extracting ViewParams
        //viewParams.isShowWork = jsonParams["view"]["isShowWork"].get<bool>();

        //Extracting SpatialSpace params...
        this->spatialState.loadFromJson(jsonParams);

        printf("GlobalParams loaded from %s\n", this->pathToJson.c_str());

    } else {
        printf("GlobalParams fail opening globalParams.json, setting default values\n");
    }
    this->setZoom(zoomParams);
    this->setView(viewParams);

    printf("+GlobalParams\n");
    
}


bool GlobalParams::addNewRelation(std::shared_ptr<FrameRelation> newFrameRelation)
{
    


    if(newFrameRelation->frame_src->frameNickName == newFrameRelation->frame_destination->frameNickName)
    {
        return false;
    }

    int existCnt = 0;
    for(int i = 0; i < this->spatialState.camFrames.size(); i++)
    {
        if((this->spatialState.camFrames[i]->frameNickName == newFrameRelation->frame_src->frameNickName)
        || (this->spatialState.camFrames[i]->frameNickName == newFrameRelation->frame_destination->frameNickName))
        {
            existCnt++;
        }
    }

    for(int i = 0; i < this->spatialState.customFrames.size(); i++)
    {
        if((this->spatialState.customFrames[i]->frameNickName == newFrameRelation->frame_src->frameNickName)
        || (this->spatialState.customFrames[i]->frameNickName == newFrameRelation->frame_destination->frameNickName))
        {
            existCnt++;
        }
    }
    printf("Exist cnt = %d\n", existCnt);

    if(existCnt == 2) // means both frames exist and are different
    {
        //1st delete all occurances that have specific nickname...
        for(int i = 0; i < this->spatialState.frameRelations.size(); i++)
        {
            if((this->spatialState.frameRelations[i]->frame_src->frameNickName == newFrameRelation->frame_src->frameNickName
            && this->spatialState.frameRelations[i]->frame_destination->frameNickName == newFrameRelation->frame_destination->frameNickName)
            || (this->spatialState.frameRelations[i]->frame_src->frameNickName == newFrameRelation->frame_destination->frameNickName
            && this->spatialState.frameRelations[i]->frame_destination->frameNickName == newFrameRelation->frame_src->frameNickName))
            {
                //means it should be deleted
                this->spatialState.frameRelations.erase(this->spatialState.frameRelations.begin() + i);
                i--;
                printf("Deleted one element from spatial space\n");
            }
        }

        //Now add new relation
        this->spatialState.frameRelations.push_back(newFrameRelation);
        printf("Added new relation, it has %d elements now\n", this->spatialState.frameRelations.size());
    }
    //newFrameRelation->frame_src
    return true;
}

GlobalParams::~GlobalParams(){
    printf("-GlobalParams\n");
}

Structs::ViewParams GlobalParams::getView(){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->view;
}
void GlobalParams::setView(Structs::ViewParams& view){
    std::lock_guard<std::mutex> guard(this->mutex);
    this->view = view;
}

std::shared_ptr<FrameBase> GlobalParams::getFrame(std::string frameName)
{
    for(int i = 0; i < this->spatialState.camFrames.size(); i++)
    {
        if(this->spatialState.camFrames[i]->frameNickName == frameName)
        {
            return this->spatialState.camFrames[i];
        }
    }

    for(int i = 0; i < this->spatialState.customFrames.size(); i++)
    {
        if(this->spatialState.customFrames[i]->frameNickName == frameName)
        {
            return this->spatialState.customFrames[i];
        }
    }
    return nullptr;
}


std::string GlobalParams::getWorldFrame()
{
    std::lock_guard<std::mutex> guard(this->mutex);

    return this->spatialState.frameWorld;
}

void GlobalParams::setWorldFrame(std::string worldFrameName)
{
   std::lock_guard<std::mutex> guard(this->mutex);

    this->spatialState.frameWorld = worldFrameName;
}


Structs::ZoomParams GlobalParams::getZoom(){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->zoom;
}
void GlobalParams::setZoom(Structs::ZoomParams& zoom){
    std::lock_guard<std::mutex> guard(this->mutex);
    this->zoom = zoom;
}

std::vector<std::shared_ptr<FrameCam>> GlobalParams::getCamFrames(){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->spatialState.camFrames;
}
std::vector<std::shared_ptr<FrameCustom>> GlobalParams::getCamCustomFrames(){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->spatialState.customFrames;
}

std::shared_ptr<FrameCustom> GlobalParams::getCustomFrame(const std::string& customNickname){
    std::lock_guard<std::mutex> guard(this->mutex);
    for(int i = 0; i < this->spatialState.customFrames.size(); i++)
    {
        if(this->spatialState.customFrames[i]->frameNickName == customNickname)
        {
            return this->spatialState.customFrames[i];
        }
    }
    return nullptr;
}


std::vector<std::shared_ptr<FrameRelation>> GlobalParams::getCamRelations(){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->spatialState.frameRelations;
}

std::shared_ptr<FrameCam> GlobalParams::openStream(const std::string& streamUrlName){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->spatialState.openStream(streamUrlName);
}

void GlobalParams::createStream(const std::string& streamUrlName, const std::string& nickName){
    std::lock_guard<std::mutex> guard(this->mutex);
    this->spatialState.createStream(streamUrlName, nickName);
}

void GlobalParams::createCustomFrame(const std::string& nickName, const std::string& cam1Nickname, const std::string& cam2Nickname){
    std::lock_guard<std::mutex> guard(this->mutex);
    this->spatialState.createCustomFrame(nickName, cam1Nickname, cam2Nickname);
}

bool GlobalParams::doesStreamExist(const std::string& streamUrlName){
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->spatialState.doesStreamExist(streamUrlName);
}
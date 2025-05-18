#pragma once

#include "FrameBase.hpp"
#include "FrameCam.hpp"
#include "FrameCustom.hpp"

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>

class SpatialState {
public:
    std::vector<std::shared_ptr<FrameCam>> camFrames; // one of them can be world frame
    std::vector<std::shared_ptr<FrameCustom>> customFrames;
    

    std::vector<std::shared_ptr<FrameRelation>> frameRelations;
    std::string frameWorld = "";


    //set all methods

    //loadFromJson
    //saveToJson
    
    void loadFromJson(nlohmann::json& json);
    void saveToJson(nlohmann::json& json);

    std::shared_ptr<FrameCam> openStream(const std::string& streamUrlName);
    void createStream(const std::string& streamUrlName, const std::string& nickname);
    void createCustomFrame(const std::string& nickName, const std::string& cam1Nickname, const std::string& cam2Nickname);
    
    bool doesStreamExist(const std::string& streamUrlName);

private:

};

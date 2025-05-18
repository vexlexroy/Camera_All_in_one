#pragma once
#include "Structs.hpp"
#include "SpatialState.hpp"
#include "FrameCam.hpp"
#include "FrameRelation.hpp"

#include <iostream>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

#include <unordered_set>

class GlobalParams {
public:
    std::unordered_set<int> openedDShowCameras;


    GlobalParams(const GlobalParams&) = delete;
    GlobalParams& operator=(const GlobalParams&) = delete;

    static GlobalParams& getInstance();

    void setView(Structs::ViewParams& view);
    Structs::ViewParams getView();

    void setZoom(Structs::ZoomParams& zoom);
    Structs::ZoomParams getZoom();


    /**
     * @brief Old relation 1->2 and 2->1 are deleted..., and new relation is inserted...
     * @return true if cam frames of new relation exists -> is able to create new entry
     */
    bool addNewRelation(std::shared_ptr<FrameRelation> newFrameRelation);


    void saveParams();
    std::vector<std::shared_ptr<FrameCam>> getCamFrames();
    std::vector<std::shared_ptr<FrameCustom>> getCamCustomFrames();
    std::vector<std::shared_ptr<FrameRelation>> getCamRelations();
    
    std::shared_ptr<FrameCam> openStream(const std::string& streamUrlName);
    void createStream(const std::string& streamUrlName, const std::string& nickName);
    void createCustomFrame(const std::string& nickName, const std::string& cam1Nickname, const std::string& cam2Nickname);
    std::shared_ptr<FrameCustom> getCustomFrame(const std::string& customNickname);
    std::string getWorldFrame();
    void setWorldFrame(std::string worldFrameName);
    bool doesStreamExist(const std::string& streamUrlName);
    std::shared_ptr<FrameBase> getFrame(std::string frameName);


private:
    //Local params
    Structs::ViewParams view;
    Structs::ZoomParams zoom;
    SpatialState spatialState;


 
    std::mutex mutex;
    
    std::string pathToJson;

    GlobalParams(const std::string& pathToJson);
    ~GlobalParams();
};

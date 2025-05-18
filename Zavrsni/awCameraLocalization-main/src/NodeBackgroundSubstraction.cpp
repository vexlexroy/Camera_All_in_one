#include "NodeBackgroundSubstraction.hpp"
#include "NodeBase.hpp"
#include "Enums.hpp"
#include "ElementFactory.hpp"
#include "Message.hpp"
#include "ConnectorBase.hpp"
#include "Util.hpp"
#include "GlobalParams.hpp"


#include <memory>
#include <vector>
#include <iostream>
#include <imgui.h>
#include <opencv2/core/matx.hpp>
#include <algorithm>
#include <imnodes.h>

NodeBackgroundSubstraction::NodeBackgroundSubstraction(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEBACKGROUNDSUBSTRACTION;
}

std::shared_ptr<NodeBase> NodeBackgroundSubstraction::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeBackgroundSubstraction>(uniqueId);
}



std::vector<Enums::MessageType> NodeBackgroundSubstraction::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeBackgroundSubstraction::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::PICTURE);
    
    return outMessageTypes;
}


std::string NodeBackgroundSubstraction::getDescription(){
    return "Ovo je background substraction node";
}

std::string NodeBackgroundSubstraction::getName(){
    return "Node-background-substraction";
}

Enums::NodeType NodeBackgroundSubstraction::getType(){
    return Enums::NodeType::NODEBACKGROUNDSUBSTRACTION;
}




void NodeBackgroundSubstraction::drawNodeParams(){
    ImGui::PushItemWidth(150);
    ImGui::Combo("type", &this->typeCurrent, this->typesStrings, IM_ARRAYSIZE(this->typesStrings));

    //capture background...
    if(ImGui::Button("capture-background")){
        this->backgroundImage = std::make_unique<cv::Mat>(this->filteredMsg->first.clone());
    }
    ImGui::PopItemWidth();
}

void NodeBackgroundSubstraction::drawNodeWork(){
    this->mutex.lock();
    if(this->filteredMsg == nullptr){
        this->mutex.unlock();
        return;
    }
    
    //select types...
    
    

    //na lijevi klik prikaži background..., na desni current frame, a bez klika na sliku prikaži filtriranu situaciju...
    

    //vidi jeli miš nad slikom, i jeli pritisnuo lijevi ili desni klik...

    bool pictureHovered = true; // get true condition if picture is being hovered

    
    if (ImGui::IsMouseDown(3) && this->backgroundImage != nullptr) {//Back button pressed
        Util::mat2Texture(*this->backgroundImage, this->texture);
    }
    else if (ImGui::IsMouseDown(4)) {//Forward button pressed
        Util::mat2Texture(this->filteredMsg->first, this->texture);
    }
    else{

        
        //Show bassicaly what has been filtered in last go
        Util::mat2Texture(this->filteredMsg->first, this->filteredMsg->second, this->texture);
        
    }
    
    ImGui::Image((ImTextureID)(intptr_t)this->texture, ImVec2(640*GlobalParams::getInstance().getZoom().scaleFactor, 480*GlobalParams::getInstance().getZoom().scaleFactor));
    
    
    
    
    this->mutex.unlock();
    return;
}



void NodeBackgroundSubstraction::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if(connector->connectorMessageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        //printf("Message came\n");
        
        
        
        this->mutex.lock();
        //std::unique_ptr<std::pair<cv::Mat, cv::Mat>>  = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));

        
        //sada dodati filtraciju po 
        if(this->backgroundImage != nullptr){

            //const char* typesStrings[8] = { "MOG", "MOG2", "KNN", "CNT", "GMG", "LSBP", "GSOC", "GSOC_TBB"};
            cv::Ptr<cv::BackgroundSubtractor> pBackSub;

            if (strcmp(typesStrings[typeCurrent], "MOG2") == 0) {
                pBackSub = cv::createBackgroundSubtractorMOG2(500, 16, true);
            }
            else if (strcmp(typesStrings[typeCurrent], "KNN") == 0) {
                pBackSub = cv::createBackgroundSubtractorKNN();
            }
            

            // Apply the background subtractor on the background and current frames
            cv::Mat fgMask;
            pBackSub->apply(*this->backgroundImage, fgMask, 1); // Initialize with background
            pBackSub->apply(msg->data->first, fgMask);       // Apply on current image

            // Update the mask based on the background subtraction result
            for (int i = 0; i < msg->data->second.rows; ++i) {
                for (int j = 0; j < msg->data->second.cols; ++j) {
                    if (msg->data->second.at<uchar>(i, j) == 255 && fgMask.at<uchar>(i, j) == 0) {
                        msg->data->second.at<uchar>(i, j) = 0;
                    }
                }
            }
            
        }
        this->filteredMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));

        this->mutex.unlock();
        

        this->sendAll(msg);
        
    } 
}


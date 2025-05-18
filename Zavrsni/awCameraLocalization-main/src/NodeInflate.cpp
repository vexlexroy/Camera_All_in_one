#include "NodeInflate.hpp"
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

NodeInflate::NodeInflate(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEINFLATE;
}

std::shared_ptr<NodeBase> NodeInflate::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeInflate>(uniqueId);
}



std::vector<Enums::MessageType> NodeInflate::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeInflate::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::PICTURE);
    
    return outMessageTypes;
}


std::string NodeInflate::getDescription(){
    return "Ovo je inflate node";
}

std::string NodeInflate::getName(){
    return "Node-inflate";
}

Enums::NodeType NodeInflate::getType(){
    return Enums::NodeType::NODEINFLATE;
}





void NodeInflate::drawNodeParams(){
    ImGui::PushItemWidth(150);
    
    
    ImGui::Combo("type", &this->typeCurrent, this->typesStrings, IM_ARRAYSIZE(this->typesStrings));
    

    //show picture...
    
    ImGui::DragInt("amount", &this->amount, 0.1, -100, 100);
    ImGui::PopItemWidth();
}

void NodeInflate::drawNodeWork(){
    this->mutex.lock();
    if(this->filteredMsg == nullptr){
        this->mutex.unlock();
        return;
    }
    

    Util::mat2Texture(this->filteredMsg->first, this->filteredMsg->second, this->texture);
    ImGui::Image((ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor));

    
    
    
    this->mutex.unlock();
    return;
}


void NodeInflate::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if(connector->connectorMessageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        //printf("Message came\n");
        this->resolution[0]=msg->data->first.cols;
        this->resolution[1]=msg->data->first.rows;
        

        //perform deflate or inflate on msg...
        cv::Mat kernel;
        if(this->typesStrings[this->typeCurrent] == "RECT"){
            kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * abs(amount) + 1, 2 * abs(amount) + 1));
        }
        else if(this->typesStrings[this->typeCurrent] == "ELLIPSE"){
            kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * abs(amount) + 1, 2 * abs(amount) + 1));
        }
        else if(this->typesStrings[this->typeCurrent] == "CROSS"){
            kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(2 * abs(amount) + 1, 2 * abs(amount) + 1));
        }
        

        if (this->amount > 0) {
            // Perform dilation (inflate)
           
            cv::dilate(msg->data->second, msg->data->second, kernel);
        } else if (this->amount < 0) {
            // Perform erosion (deflate)
            cv::erode(msg->data->second, msg->data->second, kernel);
        }

        this->mutex.lock();
        this->filteredMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));
        this->mutex.unlock();
        

        this->sendAll(msg);
        
    } 
}


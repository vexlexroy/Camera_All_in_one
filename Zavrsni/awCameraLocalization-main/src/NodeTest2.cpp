#include "NodeTest2.hpp"
#include "NodeBase.hpp"
#include "Enums.hpp"
#include "GlobalParams.hpp"

#include <memory>
#include <vector>
#include <iostream>

NodeTest2::NodeTest2(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODETEST2;
}
std::shared_ptr<NodeBase> NodeTest2::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeTest2>(uniqueId);
}
std::string NodeTest2::getDescription(){
    return "Ovo je opis pokaznog noda";
}
Enums::NodeType NodeTest2::getType(){
    return Enums::NodeType::NODETEST2;
}


std::string NodeTest2::getName(){
    return "Pokazni primjer noda";
}

std::vector<Enums::MessageType> NodeTest2::getInMessageTypes(){
    return std::vector<Enums::MessageType>{Enums::MessageType::INT};
}

std::vector<Enums::MessageType> NodeTest2::getOutMessageTypes(){
    return std::vector<Enums::MessageType>{Enums::MessageType::INT};
}

void NodeTest2::drawNodeParams(){
    if(ImGui::Button("Pritisni me!"))
        this->buttonFlag = !this->buttonFlag;
}

void NodeTest2::drawNodeWork(){
    float podatci[7] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
    if(this->buttonFlag)
        ImGui::PlotHistogram("##Histogram", podatci, IM_ARRAYSIZE(podatci)
        , 0, NULL, 0.0f, 1.0f, ImVec2(300.0f, 200.0f));
}

void NodeTest2::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    this->sendAll(message);
}


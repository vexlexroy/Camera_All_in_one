#include "ElementFactory.hpp"
#include "Enums.hpp"
#include "NodeBase.hpp"
#include "NodeTest.hpp"
#include "NodeTest2.hpp"
#include "NodeSourceStream.hpp"
#include "NodeInflate.hpp"
#include "NodeBackgroundSubstraction.hpp"
#include "ConnectorBase.hpp"
#include "ConnectorIn.hpp"
#include "ConnectorOut.hpp"
#include "ConnectionBase.hpp"
#include "Connection.hpp"
#include "NodeBlobCreator.hpp"
#include "NodeBlobGrouper.hpp"
#include "NodeManualExtrinsic.hpp"
#include "NodeDelayMesurmente.hpp"
#include "Structs.hpp"
#include "NodeArucoTracking.hpp"


#include <vector>
#include <opencv2/opencv.hpp>
#include <utility>


ElementFactory& ElementFactory::getInstance() {
    static ElementFactory elementFactoryInstance;
    return elementFactoryInstance;
}

ElementFactory::ElementFactory(){ //NODE menue
    this->uniqueIdCounter = 0;
    availableNodes.push_back(std::make_shared<NodeTest>(-1));
    availableNodes.push_back(std::make_shared<NodeTest2>(-1));
    availableNodes.push_back(std::make_shared<NodeSourceStream>(-1));
    availableNodes.push_back(std::make_shared<NodeInflate>(-1));
    availableNodes.push_back(std::make_shared<NodeBackgroundSubstraction>(-1));
    availableNodes.push_back(std::make_shared<NodeBlobCreator>(-1));
    availableNodes.push_back(std::make_shared<NodeBlobGrouper>(-1));
    availableNodes.push_back(std::make_shared<NodeManualExtrinsic>(-1));
    availableNodes.push_back(std::make_shared<NodeDelayMesurmente>(-1));
    availableNodes.push_back(std::make_shared<NodeArucoTracking>(-1));

    printf("+ElementFactory(%d nodes added)\n", (int)availableNodes.size());
}
ElementFactory::~ElementFactory(){
    printf("-ElementFactory\n");
}

std::shared_ptr<NodeBase> ElementFactory::createNewNode(Enums::NodeType nodeType) {

    for(int i = 0; i < availableNodes.size(); i++){
        if(availableNodes[i]->getType() == nodeType){
            std::shared_ptr<NodeBase> newNode = availableNodes[i]->createNewClassInstance(this->uniqueIdCounter++);
            newNode->initialize(newNode);
            return newNode;
        }
    }
    
    return nullptr;
}

std::shared_ptr<ConnectorBase> ElementFactory::createNewConnector(std::shared_ptr<NodeBase> parentNode, Enums::ConnectorDirection connectorDirection, Enums::MessageType messageType) {
    if (connectorDirection == Enums::ConnectorDirection::IN_CONNECTOR) {
        if (messageType == Enums::MessageType::INT) {
            return std::make_shared<ConnectorIn<int>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::FLOAT) {
            return std::make_shared<ConnectorIn<float>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::PICTURE){
            return std::make_shared<ConnectorIn<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::BLOB){
            return std::make_shared<ConnectorIn<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::BLOBGROUP){
            return std::make_shared<ConnectorIn<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(messageType, uniqueIdCounter++, parentNode);
        }
    } else if (connectorDirection == Enums::ConnectorDirection::OUT_CONNECTOR) {
        if (messageType == Enums::MessageType::INT) {
            return std::make_shared<ConnectorOut<int>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::FLOAT) {
            return std::make_shared<ConnectorOut<float>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::PICTURE){
            return std::make_shared<ConnectorOut<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::BLOB){
            return std::make_shared<ConnectorOut<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(messageType, uniqueIdCounter++, parentNode);
        }
        else if (messageType == Enums::MessageType::BLOBGROUP){
            return std::make_shared<ConnectorOut<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(messageType, uniqueIdCounter++, parentNode);
        }
    }
    
    return nullptr; // return empty ptr if not successfully created...
}

std::shared_ptr<ConnectionBase> ElementFactory::createNewConnection(Enums::MessageType messageType, std::shared_ptr<ConnectorBase> connectorIn, std::shared_ptr<ConnectorBase> connectorOut) {
    
    if (messageType == Enums::MessageType::INT) {
        return std::make_shared<Connection<int>>(uniqueIdCounter++, connectorIn, connectorOut);
    }
    else if (messageType == Enums::MessageType::FLOAT) {
        return std::make_shared<Connection<float>>(uniqueIdCounter++, connectorIn, connectorOut);
    }
    else if(messageType == Enums::MessageType::PICTURE){
        return std::make_shared<Connection<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(uniqueIdCounter++, connectorIn, connectorOut);
    }
    else if(messageType == Enums::MessageType::BLOB){
        return std::make_shared<Connection<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(uniqueIdCounter++, connectorIn, connectorOut);
    }
    else if(messageType == Enums::MessageType::BLOBGROUP){
        return std::make_shared<Connection<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(uniqueIdCounter++, connectorIn, connectorOut);
    }
    
    return nullptr;
}

std::vector<std::shared_ptr<NodeBase>> ElementFactory::getAvailableNodesOfInConnectorType(Enums::MessageType messageType) {
    std::vector<std::shared_ptr<NodeBase>> compatibleNodes;

    
    for (int i = 0; i < this->availableNodes.size(); i++) {
        
        if(this->availableNodes[i]->hasInMessageType(messageType)){
            compatibleNodes.push_back(this->availableNodes[i]);
        }
    }
    
    return compatibleNodes;
}

std::vector<std::shared_ptr<NodeBase>> ElementFactory::getAvailableNodesOfOutConnectorType(Enums::MessageType messageType) {
    std::vector<std::shared_ptr<NodeBase>> compatibleNodes;
    
    for (int i = 0; i < this->availableNodes.size(); i++) {        
        if(this->availableNodes[i]->hasOutMessageType(messageType)){
            compatibleNodes.push_back(this->availableNodes[i]);
        }
    }
    return compatibleNodes;
}



std::vector<std::shared_ptr<NodeBase>> ElementFactory::getAvailableNodes(){
    return availableNodes;
}
void ElementFactory::helloWorld() {
    printf("Hello world from element factory\n");
}


std::string ElementFactory::getConnectorString(Enums::MessageType messageType){
    switch(messageType){
        case Enums::MessageType::INT: return "int";
        case Enums::MessageType::FLOAT: return "float";
        case Enums::MessageType::PICTURE: return "picture";
        case Enums::MessageType::BLOB: return "blob";
        case Enums::MessageType::BLOBGROUP: return "blob-group";
        default: return "undefined";
    }
}
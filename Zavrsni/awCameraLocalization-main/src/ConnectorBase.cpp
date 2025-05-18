#include "Enums.hpp"
#include "ConnectorBase.hpp"
#include "NodeBase.hpp"
#include "ConnectionBase.hpp"


#include <memory>
#include <cstdio>


ConnectorBase::ConnectorBase(Enums::MessageType connectorMessageType, int assignedId, std::shared_ptr<NodeBase> assignedParentNode) {
    this->connectorMessageType = connectorMessageType;
    this->connectorId = assignedId;
    this->parentNode = assignedParentNode;
    // Ensure NodeBase is fully known to access nodeId
    printf("+ConnectorBase(ID=%d, PARENTID=%d)\n", this->connectorId, this->parentNode->nodeId);
}

ConnectorBase::~ConnectorBase() {
    printf("-ConnectorBase(ID=%d, PARENTID=%d)\n", this->connectorId, this->parentNode->nodeId);
    
}

void ConnectorBase::drawConnection(){
    if(this->connection == nullptr){
        return;
    }

    if(this->direction == Enums::ConnectorDirection::OUT_CONNECTOR){
        this->connection->drawConnection();
    }
    
}


void ConnectorBase::removeConnection(int connectionId){
    if(this->connection == nullptr){
        return;
    }

    if(this->connection->id == connectionId){
        
        if(this->direction == Enums::ConnectorDirection::IN_CONNECTOR){
            this->connection->connectorOut->connection = nullptr;
            this->connection->connectorIn->connection = nullptr;
        }
        else if(this->direction == Enums::ConnectorDirection::OUT_CONNECTOR){
            this->connection->connectorIn->connection = nullptr;
            this->connection->connectorOut->connection = nullptr;
        }
        
        
        
        //Why i could not do theese two operations? fakat ne kužim
        //printf("this->connection->connectorIn = nullptr; %d\n", this->connection->connectorIn->connectorId);
        //this->connection->connectorIn = nullptr; //PROBLEM
        //printf("this->connection->connectorOut = nullptr;\n");
        //this->connection->connectorOut = nullptr;

        //this->connection = nullptr;
    }
}

void ConnectorBase::addConnection(std::shared_ptr<ConnectionBase> connection){
    this->connection = connection;
    this->parentNode->connectionAdded(this->connectorId, connection->id);
}
void ConnectorBase::connectionAdded(){


}


void ConnectorBase::connectionRemoved(int connectionId){
    this->parentNode->connectionRemoved(this->connectorId, connectionId);
}



void ConnectorBase::send(std::shared_ptr<MessageBase> msg){
    if(this->direction == Enums::ConnectorDirection::IN_CONNECTOR){
        return;
    }

    if(this->connection == nullptr){
        //printf("ConnectorBase: connection does not exist\n");
        return;
    }

    if(msg->messageType != this->connectorMessageType){
        return;
    }

    msg->addTimestamp("end");
    
    this->connection->send(msg);
}


void ConnectorBase::recieve(std::shared_ptr<MessageBase> msg){
    if(this->connection == nullptr){
        //printf("ConnectorBase: connection does not exist\n");
        return;
    }

    if(msg->messageType != this->connectorMessageType){
        //printf("ConnectorBase: not same message type\n");
        return;
    }

    this->parentNode->recieve(msg, this->connectorId);
}



bool ConnectorBase::isConnected(){
    if(this->connection == nullptr){
        return false;
    }
    return true;
}
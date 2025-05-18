#pragma once

#include "ConnectorBase.hpp"
#include "Enums.hpp"

template<typename T>
class ConnectorIn: public ConnectorBase{
    public:
        ConnectorIn(Enums::MessageType connectorMessageType, int assignedId, std::shared_ptr<NodeBase> assignedParentNode);
        
        void recieve(std::shared_ptr<MessageBase> msg);
};


#include "ConnectorIn.tpp"
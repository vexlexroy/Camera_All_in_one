#pragma once

#include "ConnectorBase.hpp"
#include "Enums.hpp"

template<typename T>
class ConnectorOut: public ConnectorBase{
    public:
        ConnectorOut(Enums::MessageType connectorMessageType, int assignedId, std::shared_ptr<NodeBase> assignedParentNode);
        
};

#include "ConnectorOut.tpp"
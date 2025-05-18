#include "ConnectorOut.hpp"
#include "Enums.hpp"

template<typename T>
ConnectorOut<T>::ConnectorOut(Enums::MessageType connectorMessageType, int assignedId, std::shared_ptr<NodeBase> assignedParentNode) : ConnectorBase(connectorMessageType, assignedId, assignedParentNode){
    this->direction = Enums::ConnectorDirection::OUT_CONNECTOR;
}



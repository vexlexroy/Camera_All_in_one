#include "ConnectorIn.hpp"
#include "Enums.hpp"

template<typename T>
ConnectorIn<T>::ConnectorIn(Enums::MessageType connectorMessageType, int assignedId, std::shared_ptr<NodeBase> assignedParentNode) : ConnectorBase(connectorMessageType, assignedId, assignedParentNode) {
    this->direction = Enums::ConnectorDirection::IN_CONNECTOR;
}
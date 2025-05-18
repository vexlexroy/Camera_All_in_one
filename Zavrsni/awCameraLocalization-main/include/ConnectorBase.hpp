#pragma once

#include "Enums.hpp"
#include "MessageBase.hpp"

#include <memory>
#include <cstdio>

class NodeBase;
class ConnectionBase;

class ConnectorBase {
public:
    std::shared_ptr<NodeBase> parentNode;
    std::shared_ptr<ConnectionBase> connection;

    

    int connectorId;
    Enums::ConnectorDirection direction;
    Enums::MessageType connectorMessageType;

    ConnectorBase(Enums::MessageType connectorMessageType, int assignedId, std::shared_ptr<NodeBase> assignedParentNode);
    ~ConnectorBase();

    virtual void drawConnection();
    virtual void removeConnection(int connectionId);

    //Connection must exist, and msg type must be same of this connector type
    //If condition met message is being forwarded, otherwise nothing happens.
    void send(std::shared_ptr<MessageBase> msg);

    //Connection must exist, and msg type must be same of this connector type
    //If condition met message is being further forwarded, otherwise nothing happens.
    void recieve(std::shared_ptr<MessageBase> msg);

    void addConnection(std::shared_ptr<ConnectionBase> connection);
    void connectionAdded();

    //This is triggered when connection is removed
    void connectionRemoved(int connectorId);
    bool isConnected();
};

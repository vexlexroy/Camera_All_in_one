#pragma once

#include "Enums.hpp"
#include <vector>
#include <memory>
#include <cstdio>

// Forward declaration of ConnectorBase
class ConnectorBase;

// Definition of NodeBase class
class NodeBase {
public:
    std::vector<std::shared_ptr<ConnectorBase>> connectors;

    int nodeId;
    Enums::NodeType nodeType;
    std::vector<Enums::MessageType> inMessageTypes;
    std::vector<Enums::MessageType> outMessageTypes;

    NodeBase(int assignedId) {
        printf("Base_node build\n");
        this->nodeId = assignedId;
        this->setInMessageTypes();
        this->setOutMessageTypes();
    }

    virtual ~NodeBase() {
        printf("Base_node destroyed\n");
    }

    virtual std::shared_ptr<NodeBase> newClassInstance() = 0;
    virtual void setInMessageTypes() = 0;
    virtual void setOutMessageTypes() = 0;
};

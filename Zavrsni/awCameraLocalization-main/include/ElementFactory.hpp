#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Enums.hpp"
class NodeBase;
class NodeTest;
class NodeTest2;
class ConnectorBase;
//class ConnectorIn<T>;
//class ConnectorOut;
class ConnectorBase;
//class Connection;
class ConnectionBase;

class ElementFactory : public std::enable_shared_from_this<ElementFactory>{
private:
    int uniqueIdCounter;
    std::vector<std::shared_ptr<NodeBase>> availableNodes;

public:
    ElementFactory(const ElementFactory&) = delete;
    ElementFactory& operator=(const ElementFactory&) = delete;

    
    static ElementFactory& getInstance();


    std::shared_ptr<NodeBase> createNewNode(Enums::NodeType nodeType);

    std::shared_ptr<ConnectorBase> createNewConnector(std::shared_ptr<NodeBase> parentNode, Enums::ConnectorDirection connectorDirection, Enums::MessageType messageType);

    std::shared_ptr<ConnectionBase> createNewConnection(Enums::MessageType messageType, std::shared_ptr<ConnectorBase> connectorIn, std::shared_ptr<ConnectorBase> connectorOut);

    std::vector<std::shared_ptr<NodeBase>> getAvailableNodesOfInConnectorType(Enums::MessageType messageType);

    std::vector<std::shared_ptr<NodeBase>> getAvailableNodesOfOutConnectorType(Enums::MessageType messageType);

    std::vector<std::shared_ptr<NodeBase>> getAvailableNodes();

    static void helloWorld();


    static std::string getConnectorString(Enums::MessageType messageType);


private:
    ElementFactory(void);
    ~ElementFactory();
};

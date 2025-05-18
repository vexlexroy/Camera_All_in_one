#pragma once

#include "Enums.hpp"
#include "MessageBase.hpp"
#include "Structs.hpp"
//#include "ElementFactory.hpp"

#include <vector>
#include <memory>
#include <iostream>
#include <mutex>
#include <imgui.h>


class ConnectorBase;
class ElementFactory;

class NodeBase: public std::enable_shared_from_this<NodeBase>{

    public:
        std::vector<std::shared_ptr<ConnectorBase>> connectors;

        
        int nodeId;
        Enums::NodeType nodeType;
        std::vector<Enums::MessageType> inMessageTypes;
        std::vector<Enums::MessageType> outMessageTypes;
        //ElementFactory* factory;
        //std::shared_ptr<GlobalParams> globalParams;
        std::mutex mutex;

        bool isHovered = false;
        bool isSelected = false;

        bool showNodeWork = true;

        int resolution[2] = {640,480};
        

        NodeBase(int assignedId);
        virtual ~NodeBase();

        //void createStartingConnectors(std::shared_ptr<ElementFactory> elementFactory);
        //void addConnector(std::shared_ptr<ElementFactory> elementFactory);
        //void removeConnector(std::shared_ptr<ElementFactory> elementFactory);
        void initialize(std::shared_ptr<NodeBase> parentNode);


        std::shared_ptr<ConnectorBase> getFreeInConnector(Enums::MessageType messageType);
        std::shared_ptr<ConnectorBase> getFreeOutConnector(Enums::MessageType messageType);

        std::shared_ptr<ConnectorBase> getConnector(int connectorId);
        bool hasConnector(int connectorId);

        virtual std::shared_ptr<NodeBase> createNewClassInstance(int uniqueId) = 0;
        virtual std::vector<Enums::MessageType> getInMessageTypes() = 0;
        virtual std::vector<Enums::MessageType> getOutMessageTypes() = 0;
        virtual std::string getDescription() = 0;
        virtual std::string getName() = 0;
        virtual Enums::NodeType getType() = 0;

        bool hasInMessageType(Enums::MessageType messageType);
        bool hasOutMessageType(Enums::MessageType messageType);
        virtual void removeConnection(int connectionId);
        virtual void removeConnector(int connectorId);

        virtual void addConnector(Enums::ConnectorDirection connectorDirection , Enums::MessageType messageType);

        virtual void removeAllConnectors();

        //Removes 1.all_connections, 2.all_connectors
        virtual void removeContext();

        virtual void drawNode();
        virtual void drawConnections();



        //When new connection is added to specified connector this function will be triggered
        //Use case: if want to dinamicaly add/remove new connectors
        virtual void connectionAdded(int connectorId, int connectionId);

        //If connection is being removed from specified connector then this function will be triggered.
        //Use case: if want to dinamicaly add/remove new connectors
        virtual void connectionRemoved(int connectorId, int connectionId);
        
        //Sends message to all compatible out connectors
        virtual void sendAll(std::shared_ptr<MessageBase> message);


        //Sends message to specified connector
        virtual void send(std::shared_ptr<MessageBase> message, std::shared_ptr<ConnectorBase> connector);

        //Message is being recieved from specified connector
        virtual void recieve(std::shared_ptr<MessageBase> msg, int connectorId);

        //If direction = nullptr than it returns both in and out connector, if messageType == nullptr than it returns all messages of 
        int getConnectorCnt(Enums::ConnectorDirection direction, Enums::MessageType messageType, Enums::ConnectorState connectorState);

        int getConnectorStartCnt(Enums::ConnectorDirection direction, Enums::MessageType messageType);
        

    protected:
        ImVec2 rectangleSizeParams = ImVec2(100, 100);
        ImVec2 rectangleSizeDisplay = ImVec2(100, 100);
        

        ImVec2 getRectangleSizeConnectors(Enums::ConnectorDirection connectorDirection);

        virtual void drawNodeConnectors(Enums::ConnectorDirection connectorDirection);
        //virtual void drawNodeInConnectors();
        //virtual void drawNodeOutConnectors();
        virtual void drawNodeParams() = 0;
        virtual void drawNodeWork() = 0;

        virtual void drawNodeTitle();


        //Makes sure there is always one connector unconnected of connectorDirection start connectors
        void connectorsStrategyDynamicConnectionAdded(Enums::ConnectorDirection direction, int connectorId, int connectionId);

        void connectorsStrategyDynamicConnectionRemoved(Enums::ConnectorDirection direction, int connectorId, int connectionId);
        
};


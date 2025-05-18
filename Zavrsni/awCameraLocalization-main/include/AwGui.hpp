#pragma once


#include "Mytemplate.hpp"
#include "Structs.hpp"

#include <iostream>
#include <imgui.h>
#include <imnodes.h>
//#include "Class1.hpp"
//#include "Class2.hpp"
//#include "Connection.hpp"
#include "NodeBase.hpp"
#include "ConnectorBase.hpp"
#include "Util.hpp"
//#include "ConnectorIn.hpp"
//#include "ConnectorOut.hpp"
//#include "Class_1.hpp"
//#include "Class_base.hpp"
//#include <boost/version.hpp>
#include "ElementFactory.hpp"  // Include necessary headers
#include "GlobalParams.hpp"

class AwGui{
public:

    AwGui(const AwGui&) = delete;
    AwGui& operator=(const AwGui&) = delete;

    static AwGui& getInstance(){
        static AwGui awGuiInstance;
        return awGuiInstance;
    }

    void closeApp(void){
        GlobalParams::getInstance().saveParams();

        //remove all nodes..., i can skip this i do not have anything important in there
    }

    void process(void){
        //prvo započni Imgui prozor...
        ImGui::Begin("AwGui");
        long long int currTimestamp = Util::timeSinceEpochMicroseconds();
        double ms = ((double)(currTimestamp - this->lastTimestamp))/1000;
        double fps = 1000/ms;
        this->lastTimestamp = currTimestamp;
        ImGui::Text("Ovo je AwGui!!, %.0fFPS [%.2fms]", fps, ms);
        
        ImNodes::BeginNodeEditor();
        UserActionNodeEditor();
        DrawAllNodes();
        DrawAllConnections();

        ImNodes::MiniMap(0.3f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();
        UserActionNodeEditorPost();

        
        ImGui::End();
    }
private:
    long long int lastTimestamp = 0;
    int uniqueIdVal;
    int cnt = 0;
    int dropedFromConnectorId = 0;
    int dropedFromNodeId = 0;




    std::vector<std::shared_ptr<NodeBase>> nodes;
    
    AwGui(){
        //Init singletrons
        ElementFactory::getInstance();
        GlobalParams::getInstance();


        printf("+AwGui\n");
    }
    ~AwGui(){
        printf("-AwGui\n");
    }
    void checkNewNode(){

        bool addNodeCondition =  ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImNodes::IsEditorHovered() && (ImGui::IsKeyPressed(ImGuiKey_A) || ImGui::IsMouseClicked(ImGuiMouseButton_Right));
        //i niti jedan drugi node nije hoveran...
        

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
        if(!ImGui::IsAnyItemHovered() && addNodeCondition){
            ImGui::OpenPopup("add node");
        }

        if(ImGui::BeginPopup("add node")){
            const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

            //ImGui::SeparatorText("Add node");
            
            
            std::vector<std::shared_ptr<NodeBase>> availableNodes = ElementFactory::getInstance().getAvailableNodes();

            for(int i = 0; i < availableNodes.size(); i++){
                if(ImGui::MenuItem(availableNodes[i]->getName().c_str())){

                    //sada trebam stvoriti isti takav node
                    std::shared_ptr<NodeBase> newNode = ElementFactory::getInstance().createNewNode(availableNodes[i]->getType());
                    

                    //printf("SHARED_PTR = %d\n", newNode2.use_count());

                    nodes.push_back(newNode);

                    ImNodes::SetNodeScreenSpacePos(newNode->nodeId, click_pos);
                    //stvoriti odabran node i dodati ga u listu nodova...
                }
            }
            
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar();
    }
    std::shared_ptr<NodeBase> getNode(int nodeId){
        for(int i = 0; i < this->nodes.size(); i++){
            if(this->nodes[i]->nodeId == nodeId){
                return this->nodes[i];
            }
        }
        return nullptr;
    }

    void createNewConnection(int startNodeId, int startConnectorId, int endNodeId, int endConnectorId){
        std::shared_ptr<NodeBase> startNode = this->getNode(startNodeId);
        std::shared_ptr<NodeBase> endNode = this->getNode(endNodeId);

        if(startNode == nullptr || endNode == nullptr){
            printf("This error should not occur1\n");
            return;
        }

        std::shared_ptr<ConnectorBase> outConnector = startNode->getConnector(startConnectorId);
        std::shared_ptr<ConnectorBase> inConnector = endNode->getConnector(endConnectorId);

        if(outConnector == nullptr || inConnector == nullptr){
            printf("This error should not occur2\n");
            return;
        }

        if(outConnector->connection != nullptr || inConnector->connection != nullptr){
            printf("Currently i am not able to rewriteConnection\n");
            return;
        }

        if(outConnector->connectorMessageType != inConnector->connectorMessageType){
            printf("Can not create connection between different message types\n");
            return;
        }


        //Create new connection...
        std::shared_ptr<ConnectionBase> newConnection = ElementFactory::getInstance().createNewConnection(outConnector->connectorMessageType, inConnector, outConnector);
        
        if(newConnection == nullptr){
            printf("This error should not occur4\n");
            return;
        }

        //sami konektor je postavljen, sada još tu konekciju treba pridjeliti svakom od konektora
        
        //sada dodijeli svakom od konektroa novu konekciju.ž
        outConnector->addConnection(newConnection);
        inConnector->addConnection(newConnection);
    }
    void checkNewConnection(){
        
        
        int startNodeId, startConnectorId, endNodeId, endConnectorId;
        
        if (ImNodes::IsLinkCreated(&startNodeId, &startConnectorId, &endNodeId, &endConnectorId))
        {   
            
            createNewConnection(startNodeId, startConnectorId, endNodeId, endConnectorId);

            //Trebam basicaly vidjeti 


            /*
            const NodeType start_type = graph_.node(start_attr).type;
            const NodeType end_type = graph_.node(end_attr).type;

            const bool valid_link = start_type != end_type;
            if (valid_link)
            {
                // Ensure the edge is always directed from the value to
                // whatever produces the value
                if (start_type != NodeType::value)
                {
                    std::swap(start_attr, end_attr);
                }
                graph_.insert_edge(start_attr, end_attr);
            }
            */
        }
    
    }
    void UserActionNodeEditor(){
        //Node is hovered and focused, other window is not focused...
        checkNewNode();

    }
    
    void checkDeleteSelected(){
        /*
        ImGuiIO& io = ImGui::GetIO();
        for (int key = 0; key < IM_ARRAYSIZE(io.KeysDown); key++) {
            if (ImGui::IsKeyReleased((ImGuiKey)key)) {
                printf("Pressed = %d\n", key);
            }
        }
        */
        
        
        bool deleteSelectedCondition =  ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && (ImGui::IsKeyPressed(ImGuiKey_Delete));
        if(deleteSelectedCondition){

            //CHECK IF ANY LINK IS SELECTED...
            // Check the number of selected links
            int selectedConnectionsNum = ImNodes::NumSelectedLinks();
            if (selectedConnectionsNum > 0) {
                //printf("Broj odabranih linkova = %d\n", selectedConnectionsNum);
                std::vector<int> selectedConnections(selectedConnectionsNum);
                ImNodes::GetSelectedLinks(selectedConnections.data());
                
                for (int i = 0; i < selectedConnections.size(); i++) {
                    //remove all selected connections-ids
                    for(int j = 0; j < this->nodes.size(); j++){
                        this->nodes[j]->removeConnection(selectedConnections[i]);
                    }
                }
            }

            int selectedNodesNum = ImNodes::NumSelectedNodes();
            if (selectedNodesNum > 0) {
                //printf("Broj odabranih ndoova = %d\n", selectedNodesNum);
                std::vector<int> selectedNodes(selectedNodesNum);
                ImNodes::GetSelectedNodes(selectedNodes.data());

                
                for (int i = 0; i < selectedNodes.size(); i++) {
                    //printf("Node %d is selected\n", selectedNodes[i]);
                    //Za svaki nod prvo maknuti sve povezane linkove...
                    for(int j = 0; j < this->nodes.size(); j++){
                        if(this->nodes[j]->nodeId == selectedNodes[i]){
                            
                            this->nodes[j]->removeContext();
                            this->nodes.erase(this->nodes.begin() + j);
                            break;
                        }
                    }

                    
                }
            }
            
        }
        
    }
    int getNodeOfConnectorId(int connectorId){
        for(int i = 0; i < this->nodes.size(); i++){
            if(this->nodes[i]->hasConnector(connectorId)){
                return this->nodes[i]->nodeId;
            }
        }
        return -1;
    }
    void checkConnectionDropVoidAttach(){
        int startedAtAttribute = -1;
        bool newNodeNewConnectionCondition = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImNodes::IsLinkDropped(&startedAtAttribute, false);
        
        if(!ImGui::IsAnyItemHovered() && newNodeNewConnectionCondition){
            ImGui::OpenPopup("add node");
            this->dropedFromConnectorId = startedAtAttribute;
            this->dropedFromNodeId = this->getNodeOfConnectorId(this->dropedFromConnectorId);
            if(this->dropedFromNodeId == -1){
                printf("Ova se greška nikada nesmije pojaviti\n");
            }
        }
        if(ImGui::BeginPopup("add node")){
            const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

            //prvo: dohvati node koji započinje kom...
            std::shared_ptr<NodeBase> nodeStartDrag = this->getNode(this->dropedFromNodeId);
            std::shared_ptr<ConnectorBase> connectorStartDrag = nodeStartDrag->getConnector(this->dropedFromConnectorId);
            
            std::vector<std::shared_ptr<NodeBase>> availableNodes;
            if(connectorStartDrag->direction == Enums::ConnectorDirection::IN_CONNECTOR){
                //Onda tražim koji out nodovi podržavaju isti tip poruke...
                availableNodes = ElementFactory::getInstance().getAvailableNodesOfOutConnectorType(connectorStartDrag->connectorMessageType);
            }
            else if(connectorStartDrag->direction == Enums::ConnectorDirection::OUT_CONNECTOR){
                //Onda tražim koji in nodovi podržavaju isti tip poruke...
                availableNodes = ElementFactory::getInstance().getAvailableNodesOfInConnectorType(connectorStartDrag->connectorMessageType);
            }
            
            if(availableNodes.size() == 0){
                ImGui::TextUnformatted("No available nodes of this type\n");
            }
            

            for(int i = 0; i < availableNodes.size(); i++){
                if(ImGui::MenuItem(availableNodes[i]->getName().c_str())){

                    //sada trebam stvoriti isti takav node
                    std::shared_ptr<NodeBase> newNode = ElementFactory::getInstance().createNewNode(availableNodes[i]->getType());
                    

                    //printf("SHARED_PTR = %d\n", newNode2.use_count());

                    nodes.push_back(newNode);

                    //Connect theese two nodes...
                    if(connectorStartDrag->direction == Enums::ConnectorDirection::IN_CONNECTOR){
                        std::shared_ptr<ConnectorBase> newConnector = newNode->getFreeOutConnector(connectorStartDrag->connectorMessageType);
                        if(newConnector == nullptr){
                            printf("This error should never occur\n");
                        }
                        createNewConnection(newNode->nodeId, newConnector->connectorId, nodeStartDrag->nodeId, connectorStartDrag->connectorId);
                    }
                    else if(connectorStartDrag->direction == Enums::ConnectorDirection::OUT_CONNECTOR){
                        std::shared_ptr<ConnectorBase> newConnector = newNode->getFreeInConnector(connectorStartDrag->connectorMessageType);
                        if(newConnector == nullptr){
                            printf("This error should never occur\n");
                        }
                        createNewConnection(nodeStartDrag->nodeId, connectorStartDrag->connectorId, newNode->nodeId, newConnector->connectorId);
                    }
                    
                    ImNodes::SetNodeScreenSpacePos(newNode->nodeId, click_pos);
                    //stvoriti odabran node i dodati ga u listu nodova...
                }
            }
            
            ImGui::EndPopup();
        }
    }
    void checkConnectionDropVoidDetach(){

    }
    void applyNodeHover(){
        int hoveredNodeID = -2;
        if(!ImNodes::IsNodeHovered(&hoveredNodeID)){
            hoveredNodeID = -2;
        }

        for(int i = 0; i < this->nodes.size(); i++){
            if(this->nodes[i]->nodeId == hoveredNodeID){
                this->nodes[i]->isHovered = true;
            }
            else{
                this->nodes[i]->isHovered = false;
            }
        }
    }
    void applyNodeSelection(){
        int selectedNodesNum = ImNodes::NumSelectedNodes();
        std::vector<int> selectedNodesId(selectedNodesNum);

        ImNodes::GetSelectedNodes(selectedNodesId.data());

        for(int i = 0; i < this->nodes.size(); i++){
            this->nodes[i]->isSelected = false;

            for(int j = 0; j < selectedNodesNum; j++){
                if(this->nodes[i]->nodeId == selectedNodesId[j]){
                    this->nodes[i]->isSelected = true;
                    break;
                }
            }
        }
    }

    void applyNodeScaleFactor(){
        //Na ctrl + scroll in/out se povećava ili smanjuje scale factor svih nodova...// globalni parametar
        if(ImGui::GetIO().KeyCtrl){
            
            float wheel = ImGui::GetIO().MouseWheel;

            Structs::ZoomParams currZoom = GlobalParams::getInstance().getZoom();

            currZoom.scaleFactor += wheel*currZoom.scaleFactorSpeed;
            if(currZoom.scaleFactor < currZoom.scaleFactorMin){
                currZoom.scaleFactor = currZoom.scaleFactorMin;
            }
            else if(currZoom.scaleFactor > currZoom.scaleFactorMax){
                currZoom.scaleFactor = currZoom.scaleFactorMax;
            }

            GlobalParams::getInstance().setZoom(currZoom);
        }

    }
    void applyNodeShowWork(){
        if(ImGui::IsKeyPressed(ImGuiKey_S)){
            Structs::ViewParams currView = GlobalParams::getInstance().getView();
            currView.isShowWork = !currView.isShowWork;
            GlobalParams::getInstance().setView(currView);
        }
    }

    void UserActionNodeEditorPost(){
        checkNewConnection();
        checkDeleteSelected();

        checkConnectionDropVoidDetach();
        checkConnectionDropVoidAttach();

        applyNodeHover();
        applyNodeSelection();

        applyNodeScaleFactor();
        applyNodeShowWork();

        int linkId = 0;

        ImNodes::IsLinkHovered(&linkId);


        int pinId = 0;

        ImNodes::IsPinHovered(&pinId);
        //printf("Link id = %d, Pin id = %d\n", linkId, pinId);

    }

    void DrawAllNodes(){
        for(int i = 0; i < this->nodes.size(); i++){
            this->nodes[i]->drawNode();
        }
    }
    void DrawAllConnections(){
        for(int i = 0; i < this->nodes.size(); i++){
            this->nodes[i]->drawConnections();
        }
    }

};
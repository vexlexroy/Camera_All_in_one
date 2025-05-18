#include "Enums.hpp"
#include "NodeBase.hpp"
#include "ConnectorBase.hpp"
#include "ConnectionBase.hpp"
#include "ElementFactory.hpp"
#include "Util.hpp"
#include "GlobalParams.hpp"

#include <stdio.h>
#include <memory>
#include <ImNodes.h>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <algorithm>

NodeBase::NodeBase(int assignedId){
    this->nodeId = assignedId;
    printf("+NodeBase(ID = %d)\n", this->nodeId);
}
NodeBase::~NodeBase(){
    printf("-NodeBase(ID = %d)\n", this->nodeId);
}

void NodeBase::initialize(std::shared_ptr<NodeBase> parentNode){
    
    //this->factory = elementFactory;
    
    //CREATING IN CONNECTORS
    std::vector<Enums::MessageType> inConnectors = this->getInMessageTypes();
    for(int i = 0; i < inConnectors.size(); i++){
        
        std::shared_ptr<ConnectorBase> newConnector = ElementFactory::getInstance().createNewConnector(parentNode, Enums::ConnectorDirection::IN_CONNECTOR, inConnectors[i]);
        this->connectors.push_back(newConnector);
    }
    //CREATING OUT CONNECTORS
    std::vector<Enums::MessageType> outConnectors = this->getOutMessageTypes();
    for(int i = 0; i < outConnectors.size(); i++){
        std::shared_ptr<ConnectorBase> newConnector = ElementFactory::getInstance().createNewConnector(parentNode, Enums::ConnectorDirection::OUT_CONNECTOR, outConnectors[i]);
        this->connectors.push_back(newConnector);
    }
}

bool NodeBase::hasInMessageType(Enums::MessageType messageType){// CHECKS IF HTERE IS MESSAGE OF CERTAIN TYPE
    std::vector<Enums::MessageType> availableTypes = this->getInMessageTypes();
    
    for(int i = 0; i < availableTypes.size(); i++){
        if(availableTypes[i] == messageType){
            return true;
        }
    }
    return false;
}


bool NodeBase::hasOutMessageType(Enums::MessageType messageType){
    std::vector<Enums::MessageType> availableTypes = this->getOutMessageTypes();

    for(int i = 0; i < availableTypes.size(); i++){
        if(availableTypes[i] == messageType){
            return true;
        }
    }
    return false;
}


void NodeBase::drawNode(){
    this->showNodeWork =  GlobalParams::getInstance().getView().isShowWork || this->isHovered || this->isSelected;
    /*
    ImVec2 oldWindowPadding = ImGui::GetStyle().WindowPadding;
    ImVec2 oldFramePadding = ImGui::GetStyle().FramePadding;

    // Set padding to zero
    ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
    ImGui::GetStyle().FramePadding = ImVec2(0, 0);
    */

    


    ImNodes::BeginNode(this->nodeId);
    
    this->drawNodeTitle();

    
    
    //Dohvati sve veličine koje trebaš dohvatiti za obilježavanja veličina childrena...
    ImVec2 rectangleSizeInConnectors = this->getRectangleSizeConnectors(Enums::ConnectorDirection::IN_CONNECTOR);
    ImVec2 rectangleSizeOutConnectors = this->getRectangleSizeConnectors(Enums::ConnectorDirection::OUT_CONNECTOR);
    ImVec2 rectangleSizeDisplay = this->rectangleSizeDisplay;
    ImVec2 rectangleSizeParams = this->rectangleSizeParams;



    ImVec2 dummy;
    dummy.y = rectangleSizeInConnectors.y;
    if(rectangleSizeOutConnectors.y > dummy.y){
        dummy.y = rectangleSizeOutConnectors.y;
    }
    if(rectangleSizeParams.y > dummy.y){
        dummy.y = rectangleSizeParams.y;
    }

    dummy.x = rectangleSizeDisplay.x - (rectangleSizeInConnectors.x + rectangleSizeOutConnectors.x + rectangleSizeParams.x);
    dummy.x /= 2;

    //printf("x = %.2f, y = %.2f\n", dummy.x, dummy.y);
    bool dummyUp = true;

    //printf("%.2f, %.2f\n", rectangleSizeDisplay.x, rectangleSizeInConnectors.x + dummy.x + dummy.x + rectangleSizeParams.x + rectangleSizeOutConnectors.x);


    if(dummy.x < 0){
        dummyUp = false;
        dummy.x = -dummy.x;
    }
    else{
        dummy.x = static_cast<int>(dummy.x) - 16;
    }


    

    //Ako je dummy.x < 0, znači da dummy treba dodati u gornji tabor (inače u gornji)

    
    //###################################GORNJA_LOŽA##########################################
    //IN CONNECTORS
    if(rectangleSizeInConnectors.x > 0 && rectangleSizeInConnectors.y > 0){
        //ImGui::BeginChild("##in_connectors", ImVec2(rectangleSizeInConnectors.x, rectangleSizeInConnectors.y), true);
        ImGui::BeginGroup();
        this->drawNodeConnectors(Enums::ConnectorDirection::IN_CONNECTOR);
        ImGui::EndGroup();
        //ImGui::EndChild();
        ImGui::SameLine();
    }
    

    //DUMMY if necessery
    
    if(dummyUp){
        ImGui::Dummy(dummy);
        ImGui::SameLine();
    }
    
    
    //LOCAL PARAMS
    
    //ImGui::BeginChild("##local_params", ImVec2(rectangleSizeParams.x, rectangleSizeParams.y), true);
    ImGui::BeginGroup(); 
    if(this->showNodeWork){
        this->drawNodeParams();
    }
    ImGui::EndGroup();
    
    //set this->rectangleSizeParams if must...
    ImVec2 localParamsMaxSize = ImGui::GetItemRectMax();
    ImVec2 localParamsMinSize = ImGui::GetItemRectMin();
    int padding = 18;
    ImVec2 localParamsOcupiedSpace;
    localParamsOcupiedSpace.x = (localParamsMaxSize.x - localParamsMinSize.x);// + padding;
    localParamsOcupiedSpace.y = (localParamsMaxSize.y - localParamsMinSize.y);// + padding;    
    if(std::abs(localParamsOcupiedSpace.x - rectangleSizeParams.x) > 3){
        this->rectangleSizeParams.x = localParamsOcupiedSpace.x;
    }
    if(std::abs(localParamsOcupiedSpace.y - rectangleSizeParams.y) > 3){
        this->rectangleSizeParams.y = localParamsOcupiedSpace.y;
    }
    //ImGui::EndChild();
    
    //DUMMY if necessery
    
    if(dummyUp){
        ImGui::SameLine();
        ImGui::Dummy(dummy);
    }

    //OUT CONNECTORS
    ImGui::SameLine();
    //ImGui::BeginChild("##out_connectors", ImVec2(rectangleSizeOutConnectors.x, rectangleSizeOutConnectors.y), true);
    ImGui::BeginGroup();
    this->drawNodeConnectors(Enums::ConnectorDirection::OUT_CONNECTOR);
    ImGui::EndGroup();
    //ImGui::EndChild();

    
    //###################################DONJA_LOŽA##########################################
    
    if(!dummyUp){
        ImGui::Dummy(dummy);
        ImGui::SameLine();
    }

    
    
    //ImGui::BeginChild("##display_work", ImVec2(rectangleSizeDisplay.x, rectangleSizeDisplay.y), true);
    ImGui::BeginGroup();
    //printf("%f, HOVERED=%d, SELECTED=%d, GLOBALWORKSHOW=%d\n", globalParams.zoom.scaleFactor, this->isHovered, this->isSelected, globalParams.view.isShowWork);
    if(this->showNodeWork){
        this->drawNodeWork();
    }
    ImGui::EndGroup();
    //set this->rectangleSizeParams if must...
    ImVec2 displayMaxSize = ImGui::GetItemRectMax();
    ImVec2 displayMinSize = ImGui::GetItemRectMin();
    padding = 18;
    ImVec2 displayOcupiedSpace;
    displayOcupiedSpace.x = displayMaxSize.x - displayMinSize.x;// + padding;
    displayOcupiedSpace.y = displayMaxSize.y - displayMinSize.y;// + padding;
    //printf("%.2f, %.2f, %.2f, %.2f\n", displayOcupiedSpace.x, displayOcupiedSpace.y, rectangleSizeDisplay.x, rectangleSizeDisplay.y);
    
    if(std::abs(displayOcupiedSpace.x - rectangleSizeDisplay.x) > 3){
        this->rectangleSizeDisplay.x = displayOcupiedSpace.x;
    }
    if(std::abs(displayOcupiedSpace.y - rectangleSizeDisplay.y) > 3){
        this->rectangleSizeDisplay.y = displayOcupiedSpace.y;
    }
    
    //ImGui::EndChild();
    
    //##########-END

    ImNodes::EndNode();
    
    /*
    // Left segment
    ImGui::BeginChild("Left Segment", ImVec2(100, 100), true, ImGuiWindowFlags_NoBackground);
    ImGui::Text("Left Segment");
    //this->drawNodeConnectors();
    ImGui::EndChild();
    // Add your content here for the left segment

    

    

    ImGui::SameLine();

    // Middle segment
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.6f, 0.3f, 0.2f, 0.7f)); // Set background color dynamically

    ImGui::BeginChild("Middle Segment", ImVec2(300, 200), true);
    ImGui::Button("ačsjdf");
    ImGui::Button("ačsjdf1");
    ImGui::Button("ačsjdf2");
    ImGui::Button("ačsjdf3");
    ImGui::Button("ačsjdf4");
    ImGui::Button("ačsjdf5");
    ImGui::Button("123456789antunweberlovroweber");
    ImGui::Button("ačsjdf7");
    ImGui::Button("ačsjdf8");
    ImGui::Button("ačsjdf9");
    ImGui::Button("ačsjdf10");
    
    ImGui::EndChild();
    ImGui::PopStyleColor();


    ImGui::SameLine();

    ImGui::BeginChild("Right Segment", ImVec2(300, 200), true, ImGuiWindowFlags_NoBackground);
    ImGui::Text("Right Segment");
    // Add your content here for the left segment

    ImGui::EndChild();
    
    //ImGui::EndChild();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    static int sizeChild = 500;
    ImGui::DragInt("sizeChild", &sizeChild);

    static int lenWidth = 10;
    ImGui::DragInt("lenWidth", &lenWidth);

    static int lenHeight = 10;
    ImGui::DragInt("lenHeight", &lenHeight);


    ImGui::BeginChild("ChildWindow", ImVec2(sizeChild, sizeChild), false, ImGuiWindowFlags_NoScrollbar);

    ImGui::BeginGroup();
    
    std::string str = "";
    for(int i = 0; i < lenWidth; i++){
        str += "a";
    }
    
    // Add content to the child window
    for(int i = 0; i < lenHeight; i++){
        ImGui::Button(str.c_str());
    }
    ImGui::Button("abc");
    
    
    ImGui::EndGroup();
    // Retrieve the size of the content region
    ImVec2 test1 = ImGui::GetContentRegionMax();
    ImVec2 test2 = ImGui::GetWindowContentRegionMax();
    ImVec2 test3 =ImGui::GetContentRegionAvail();
    ImVec2 test4 =ImGui::GetWindowContentRegionMin();
    ImVec2 test5 = ImGui::GetItemRectMax();
    ImVec2 test55 = ImGui::GetItemRectMin();
    ImVec2 test6 = ImGui::GetCursorScreenPos();
    float test7 = ImGui::GetScrollMaxX();
    float test8 = ImGui::GetScrollMaxY();
    float test11 = ImGui::GetColumnWidth(1);
    float test12 = ImGui::GetWindowWidth();

    float test9 = ImGui::GetScrollX();
    float test10 = ImGui::GetScrollY();
    float test13 =ImGui::CalcItemWidth();
    
    // End the child window
    ImGui::EndChild();
    
    ImGui::PopStyleVar();

    ImGui::Text("%.2f, %.2f" , test1.x, test1.y);
    ImGui::Text("%.2f, %.2f" , test2.x, test2.y);
    ImGui::Text("%.2f, %.2f" , test3.x, test3.y);
    ImGui::Text("%.2f, %.2f" , test4.x, test4.y);
    ImGui::Text("                                     %.2f, %.2f" , test5.x, test5.y);
    ImGui::Text("                                     %.2f, %.2f" , test55.x, test55.y);
    ImGui::Text("                                     %.2f, %.2f" , test5.x - test55.x, test5.y - test55.y);
    ImGui::Text("                                     %.2f, %.2f" , test6.x, test6.y);
    ImGui::Text("                                     %.2f, %.2f" , test6.x - test5.x, test6.y - test5.y);
    ImGui::Text("%.2f" , test7);
    ImGui::Text("%.2f" , test8);
    ImGui::Text("%.2f" , test9);
    ImGui::Text("%.2f" , test10);
    ImGui::Text("%.2f" , test11);
    ImGui::Text("%.2f" , test12);
    ImGui::Text("%.2f" , test13);

    

    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(1 / 7.0f, 0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(1 / 7.0f, 0.7f, 0.5f));
    this->drawNodeWork();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    */
    
    //ImGui::GetStyle().WindowPadding = oldWindowPadding;
    //ImGui::GetStyle().FramePadding = oldFramePadding;
}

void NodeBase::drawNodeParams(){
    ImGui::Text("Override: NodeBase::drawNodeParams");
}
void NodeBase::drawNodeWork(){
    ImGui::Text("Override: NodeBase::drawNodeWork");
}

void NodeBase::drawNodeTitle(){
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(this->getName().c_str());
    ImNodes::EndNodeTitleBar();
}
void NodeBase::drawNodeConnectors(Enums::ConnectorDirection connectorDirection){
    ImVec4 connectorColor(150.0/255.0, 150.0/255.0, 150.0/255.0, 255.0/255.0);

    for(int i = 0; i < this->connectors.size(); i++){
        std::shared_ptr<ConnectorBase> connector = this->connectors[i];
        
        if(connector->direction == connectorDirection){
            if(connectorDirection == Enums::ConnectorDirection::IN_CONNECTOR){
                ImNodes::BeginInputAttribute(connector->connectorId);
                ImGui::TextColored(connectorColor, ElementFactory::getConnectorString(connector->connectorMessageType).c_str());
                ImNodes::EndInputAttribute();
            }
            else if(connectorDirection == Enums::ConnectorDirection::OUT_CONNECTOR){
                ImNodes::BeginOutputAttribute(connector->connectorId);
                ImGui::TextColored(connectorColor, ElementFactory::getConnectorString(connector->connectorMessageType).c_str());
                ImNodes::EndOutputAttribute();
            }
        }

    }
}

void NodeBase::drawConnections(){
    
    //void for now, implement...
    for(int i = 0; i < this->connectors.size(); i++){
        this->connectors[i]->drawConnection();
    }
}


std::shared_ptr<ConnectorBase> NodeBase::getConnector(int connectorId){
    for(int i = 0; i < this->connectors.size(); i++){
        if(this->connectors[i]->connectorId == connectorId){
            return this->connectors[i];
        }
    }
    return nullptr;
}

void NodeBase::removeConnection(int connectionId){
    //Samo ako je out connector može gasiti konekciju
    for(int i = 0; i < this->connectors.size(); i++){
        this->connectors[i]->removeConnection(connectionId);
    }
}
bool NodeBase::hasConnector(int connectorId){
    
    for(int i = 0; i < this->connectors.size(); i++){
        if(this->connectors[i]->connectorId == connectorId){
            return true;
        }
    }
    return false;
}
void NodeBase::removeConnector(int connectorId){
    for(int i = 0; i < this->connectors.size(); i++){
        std::shared_ptr<ConnectorBase> connector = this->connectors[i];
        if(connector->connectorId == connectorId){
            //REMOVE CONNECTION CONTEXT IF EXISTS
            if(connector->connection != nullptr){
                printf("Removing connection...\n");
                connector->removeConnection(connector->connection->id);
                printf("Should be removed...\n");
            }

            //REMOVE CONNECTOR ITSELF (Only shared_ptr is in node)
            this->connectors.erase(this->connectors.begin() + i);
            
            return;
        }
    }
}
void NodeBase::removeAllConnectors(){

    while(this->connectors.size() > 0){
        this->removeConnector(this->connectors[0]->connectorId);
    }
   
}


void NodeBase::removeContext(){
    removeAllConnectors();
}

std::shared_ptr<ConnectorBase> NodeBase::getFreeInConnector(Enums::MessageType messageType){
    for(int i = 0; i < this->connectors.size(); i++){
        if((this->connectors[i]->direction == Enums::ConnectorDirection::IN_CONNECTOR)
        && (this->connectors[i]->connectorMessageType == messageType)
        && (this->connectors[i]->connection == nullptr)){
            return this->connectors[i];
        }
    }
    return nullptr;
}

std::shared_ptr<ConnectorBase> NodeBase::getFreeOutConnector(Enums::MessageType messageType){
    for(int i = 0; i < this->connectors.size(); i++){
        if((this->connectors[i]->direction == Enums::ConnectorDirection::OUT_CONNECTOR)
        && (this->connectors[i]->connectorMessageType == messageType)
        && (this->connectors[i]->connection == nullptr)){
            return this->connectors[i];
        }
    }
    return nullptr;
}



void NodeBase::connectionAdded(int connectorId, int connectionId){
    this->connectorsStrategyDynamicConnectionAdded(Enums::ConnectorDirection::ALL , connectorId, connectionId);

}

void NodeBase::connectionRemoved(int connectorId, int connectionId){
    this->connectorsStrategyDynamicConnectionRemoved(Enums::ConnectorDirection::ALL, connectorId, connectionId);
}


void NodeBase::sendAll(std::shared_ptr<MessageBase> message){
    for(int i = 0; i < this->connectors.size(); i++){
        this->connectors[i]->send(message->clone(message));
    }
}

void NodeBase::send(std::shared_ptr<MessageBase> message, std::shared_ptr<ConnectorBase> connector){
    connector->send(message);
}


void NodeBase::recieve(std::shared_ptr<MessageBase> msg, int connectorId){}


void NodeBase::addConnector(Enums::ConnectorDirection connectorDirection , Enums::MessageType messageType){
    //this->ElementFactory::getInstance().createNewConnector()
    std::shared_ptr<NodeBase> sharedThis = shared_from_this();
    std::shared_ptr<ConnectorBase> newConnector = ElementFactory::getInstance().createNewConnector(sharedThis, connectorDirection, messageType);
    this->connectors.push_back(newConnector);
}

//Get current connectors cnt based on direction, messageType and isFreeFilter
//If isFreeFilter == false, than it returns all
int NodeBase::getConnectorCnt(Enums::ConnectorDirection direction, Enums::MessageType messageType, Enums::ConnectorState connectorState){
    
    int cnt = 0;

    for(int i = 0; i < this->connectors.size(); i++){
        std::shared_ptr<ConnectorBase> connector = this->connectors[i];

        //SAMO AKO 
        if(connector->direction == direction || direction == Enums::ConnectorDirection::ALL){
            if(connector->connectorMessageType == messageType || messageType == Enums::MessageType::ALL){
                
                if(connectorState == Enums::ConnectorState::CONNECTED){
                    if(connector->isConnected()){
                        cnt++;
                    }
                }
                else if(connectorState == Enums::ConnectorState::UNCONNECTED){
                    if(!connector->isConnected()){
                        cnt++;
                    }
                }
                else if(connectorState == Enums::ConnectorState::ALL){
                    cnt++;
                }
            }
        }
    }
    
   return cnt;
}


//Get start connectors cnt (with what connectors node starts) based on direction and messageType
int NodeBase::getConnectorStartCnt(Enums::ConnectorDirection direction, Enums::MessageType messageType){
    int cnt = 0;

    if(direction == Enums::ConnectorDirection::IN_CONNECTOR || direction == Enums::ConnectorDirection::ALL){
        std::vector<Enums::MessageType> inMessageTypes = this->getInMessageTypes();
        for(int i = 0; i < inMessageTypes.size(); i++){
            Enums::MessageType messageTypeSet = inMessageTypes[i];

            if(messageTypeSet == messageType || messageType == Enums::MessageType::ALL){
                cnt++;
            }
        }
    }
    if(direction == Enums::ConnectorDirection::OUT_CONNECTOR || direction == Enums::ConnectorDirection::ALL){
        std::vector<Enums::MessageType> outMessageTypes = this->getOutMessageTypes();
        for(int i = 0; i < outMessageTypes.size(); i++){
            Enums::MessageType messageTypeSet = outMessageTypes[i];

            if(messageTypeSet == messageType || messageType == Enums::MessageType::ALL){
                cnt++;
            }
        }
    }

    return cnt;
}


void NodeBase::connectorsStrategyDynamicConnectionAdded(Enums::ConnectorDirection direction, int connectorId, int connectionId){

    
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if(connector->direction != direction && (direction != Enums::ConnectorDirection::ALL)){
        return;
    }
    int freeConnectorsNum = this->getConnectorCnt(connector->direction, connector->connectorMessageType, Enums::ConnectorState::UNCONNECTED);

    if(freeConnectorsNum == 0){
        this->addConnector(connector->direction, connector->connectorMessageType);
    }
}
void NodeBase::connectorsStrategyDynamicConnectionRemoved(Enums::ConnectorDirection direction, int connectorId, int connectionId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);


    if(connector->direction != direction && (direction != Enums::ConnectorDirection::ALL)){
        return;
    }


    int startConnectorsNum = this->getConnectorStartCnt(connector->direction, connector->connectorMessageType);
    int freeConnectorsNum = this->getConnectorCnt(connector->direction, connector->connectorMessageType, Enums::ConnectorState::UNCONNECTED);
    int allConnectorsNum = this->getConnectorCnt(connector->direction, connector->connectorMessageType, Enums::ConnectorState::ALL);

    //Must remove connector...
    if(freeConnectorsNum > 1 && allConnectorsNum > startConnectorsNum){
        this->removeConnector(connector->connectorId);
    }
}

ImVec2 NodeBase::getRectangleSizeConnectors(Enums::ConnectorDirection connectorDirection){
    int charWidthPadding = 20;
    int charHeightPadding = 20;

    int charWidth = 7;
    int charHeight = 19;
    
    ImVec2 minSpaningRectangle(0, 0);
    for(int i = 0; i < this->connectors.size(); i++){
        if(this->connectors[i]->direction == connectorDirection){
            if(ElementFactory::getInstance().getConnectorString(connectors[i]->connectorMessageType).size() > minSpaningRectangle.x){
                minSpaningRectangle.x = ElementFactory::getInstance().getConnectorString(connectors[i]->connectorMessageType).size();
            }
            minSpaningRectangle.y++;
        }
    }
    if(minSpaningRectangle.x != 0 || minSpaningRectangle.y != 0){
        minSpaningRectangle.x = minSpaningRectangle.x*charWidth;// + charWidthPadding;
        minSpaningRectangle.y = minSpaningRectangle.y*charHeight;// + charHeightPadding;
    }

    return minSpaningRectangle;
}


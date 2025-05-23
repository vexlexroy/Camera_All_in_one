#include "NodeArucoTracking.hpp"
#include "NodeBase.hpp"
#include "Enums.hpp"
#include "ElementFactory.hpp"
#include "Message.hpp"
#include "ConnectorBase.hpp"
#include "Util.hpp"
#include "GlobalParams.hpp"



#include <memory>
#include <vector>
#include <iostream>
#include <imgui.h>
#include <opencv2/core/matx.hpp>
#include <algorithm>

NodeArucoTracking::NodeArucoTracking(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEARUCOTRACKING;
}

std::shared_ptr<NodeBase> NodeArucoTracking::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeArucoTracking>(uniqueId);
}



std::vector<Enums::MessageType> NodeArucoTracking::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeArucoTracking::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    
    return outMessageTypes;
}


std::string NodeArucoTracking::getDescription(){
    return "This node does aruco tracking acros multiple cameras";
}

std::string NodeArucoTracking::getName(){
    return "Node-aruco-tracking";
}

Enums::NodeType NodeArucoTracking::getType(){
    return Enums::NodeType::NODEARUCOTRACKING;
}

void NodeArucoTracking::drawNodeParams(){
    
   
}

void NodeArucoTracking::drawNodeWork(){
    this->mutex.lock();
    if(this->lastMsg == nullptr){
        ImGui::PushItemWidth(100);
        ImGui::Text("world frame:");
        ImGui::SameLine();
        this->drawWorldSelector();
        ImGui::Text("view camera:");
        ImGui::SameLine();
        this->drawDropdownSelector();
        ImGui::PopItemWidth();
        this->mutex.unlock();
        return;
    }
    // get all opend streams and draw dropdown
    ImGui::PushItemWidth(100);
    ImGui::Text("Marker Size [mm]:");
    ImGui::SameLine();
    ImGui::DragFloat("##marker_size",&this->markerSize,0.05);
    ImGui::Text("world frame:");
    ImGui::SameLine();
    this->drawWorldSelector();
    ImGui::Text("view camera:");
    ImGui::SameLine();
    this->drawDropdownSelector();
    ImGui::PopItemWidth();

    //Tu update napraviti tako da se uzima freezed frame ako treba...
    Util::mat2Texture(this->lastMsg->first, this->lastMsg->second, this->texture); // if freezed -> update....

    //printf("texture id = %d\n", this->texture);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    //ImGui::ImageButton()
    if (ImGui::ImageButton("", (ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor))){
        ImGui::OpenPopup("choose range");
    }
    ImGui::PopStyleColor();
    if (ImGui::BeginPopup("choose range")){
        ImGui::SeparatorText("choose range");
        ImGuiIO& io = ImGui::GetIO();

        ImVec2 pos = ImGui::GetCursorScreenPos();

        int pictureX = io.MousePos.x - pos.x;
        int pictureY = io.MousePos.y - pos.y;
        
        ImVec2 zoom_T = ImVec2(((float)(pictureX))/this->resolution[0],((float)(pictureY))/this->resolution[1]);
        zoom_T.x = this->zoom_A.x + zoom_T.x * (this->zoom_B.x - this->zoom_A.x);
        zoom_T.y = this->zoom_A.y + zoom_T.y * (this->zoom_B.y - this->zoom_A.y);

        //Window mora biti hoveran, keyShift mor
        bool windowHovered = false;
        bool shiftHolded = false;
        bool shiftClicked = false;
        bool zoomWheelMoved = false;
        float zoomLevel = this->zoom_B.x - this->zoom_A.x;


        if((zoom_T.x >= 0) && (zoom_T.x <= 1) && (zoom_T.y >= 0) && (zoom_T.y <= 1))
        {
            windowHovered = true;
        }

        if (ImGui::GetIO().KeyShift) 
        {
            if(this->isShiftPresed == false)
            {
                shiftClicked = true;
            }
            this->isShiftPresed = true;
            shiftHolded = true;
        }
        else{
            this->isShiftPresed = false;
        }

        if(ImGui::GetIO().MouseWheel != 0.0f)
        {
            zoomWheelMoved = true;
        }

        if(windowHovered && shiftClicked) // freeze the image
        {
            freezeFrameFlag = true;
            //this->freezedFrame =  hsvFrame.clone();
            //printf("Freeze!\n");
        }

        if(windowHovered && shiftHolded && zoomWheelMoved)
        {
            if ((ImGui::GetIO().MouseWheel > 0.0f) && (zoomLevel > 0.05)) { // zoom in critera

                //By how much will i zoom in/out -> every zoom in i zoom by tenth of previous value

                zoomLevel = zoomLevel * 0.9;

                //Recalculate new zoom level
                zoom_A.x = zoom_T.x - zoomLevel * ((zoom_T.x - zoom_A.x)/(zoom_B.x - zoom_A.x)); // zoom_A.x = zoom_T.x - zoom_T.x*zoomLevel;
                zoom_B.x = zoom_A.x + zoomLevel;

                zoom_A.y = zoom_T.y - zoomLevel * ((zoom_T.y - zoom_A.y)/(zoom_B.y - zoom_A.y)); // zoom_A.y = zoom_T.y - zoom_T.y*zoomLevel;
                zoom_B.y = zoom_A.y + zoomLevel;


                //printf("Scrool up, zoomLevel = %.2f\n", zoomLevel);
                //printf("zoom_T = %.2f, %.2f\n", zoom_T.x, zoom_T.y);
                //printf("zoom_A = %.2f, %.2f\n", zoom_A.x, zoom_A.y);
                //printf("zoom_B = %.2f, %.2f\n", zoom_B.x, zoom_B.y);

            } else { // zoom out criteria
                //printf("Scrool down\n");
                zoomLevel = zoomLevel * 1.1;
                if(zoomLevel > 1)
                {
                    zoomLevel = 1;
                }

                zoom_A.x = zoom_T.x - zoomLevel * ((zoom_T.x - zoom_A.x)/(zoom_B.x - zoom_A.x)); // zoom_A.x = zoom_T.x - zoom_T.x*zoomLevel;
                zoom_B.x = zoom_A.x + zoomLevel;

                zoom_A.y = zoom_T.y - zoomLevel * ((zoom_T.y - zoom_A.y)/(zoom_B.y - zoom_A.y)); // zoom_A.y = zoom_T.y - zoom_T.y*zoomLevel;
                zoom_B.y = zoom_A.y + zoomLevel;

                //provjeriti granice... i pomaknuti ih u skladu....
                if(zoom_A.x < 0)
                {
                    zoom_B.x += -zoom_A.x;
                    zoom_A.x = 0;
                }
                
                if(zoom_B.x > 1)
                {
                    zoom_A.x -= zoom_B.x - 1;
                    zoom_B.x = 1;
                }

                if(zoom_A.y < 0)
                {
                    zoom_B.y += -zoom_A.y;
                    zoom_A.y = 0;
                }
                if(zoom_B.y > 1)
                {
                    zoom_A.y -= zoom_B.y - 1;
                    zoom_B.y = 1;
                }

            }
        }
        
        //T scaled cordinates must stay consistant -> if there is scaling -> it must be updated to represent correct value
        //This should be correct texture
        Util::mat2Texture(this->lastMsg->first, this->lastMsg->second, this->selectTexture);
        ImGui::Image((ImTextureID)(intptr_t)this->selectTexture, ImVec2(this->resolution[0], this->resolution[1]), this->zoom_A, this->zoom_B);
        
        ImGui::EndPopup();
    }

    

    this->mutex.unlock();
    return;
}



void NodeArucoTracking::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);
    if(connector->connectorMessageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        this->resolution[0]=msg->data->first.cols;
        this->resolution[1]=msg->data->first.rows;
        std::string camName = msg->camOrigin->frameNickName;

        this->mutex.lock();
        if(!this->isShiftPresed)
        {
            if(msg->camOrigin->frameNickName == this->selectedCameraName){
                this->lastMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone())); // nije međudretveno sigurno
            }
            // else{
            //     std::cout << "sel: " << this->selectedCameraName << "got: " << msg->camOrigin->frameNickName << "\n";
            // }

        }
        this->mutex.unlock();  
    }
    
}

void NodeArucoTracking::getConCams(){
    // Get all active camera frames
    std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
    this->availableCameras.clear();
    // Populate available cameras
    for (const auto& cam : camFrames) {
        if (cam->isConnected) {
            this->availableCameras.push_back(cam->frameNickName);
            // printf("2");
        }
        // printf("3");
    }
    return;
}

void NodeArucoTracking::getValWorlds(){
    this->availableWorlds.clear();
    this->getConCams();
    std::vector<std::shared_ptr<FrameCustom>> customFrames = GlobalParams::getInstance().getCamCustomFrames();
    std::vector<std::shared_ptr<FrameRelation>> relations = GlobalParams::getInstance().getCamRelations();
    for (const auto& frame : customFrames) {
        for(auto& rels : relations){
            for(auto& cam : this->availableCameras){
                if(
                    (cam == rels->frame_destination->frameNickName || cam == rels->frame_src->frameNickName)
                    && (frame->frameNickName == rels->frame_destination->frameNickName || frame->frameNickName == rels->frame_src->frameNickName)
                ){
                    this->availableWorlds.push_back(frame->frameNickName);
                }
            }
        }
    }
    for(auto cam : this->availableCameras){
        this->availableWorlds.push_back(cam);
    }
    return;
}

void NodeArucoTracking::drawDropdownSelector(){
    this->getConCams();
    // If no cameras are available, show a message
    if (this->availableCameras.empty()) {
        ImGui::Text("No connected cameras available");
        return;
    }
    
    // Ensure selected index is valid
    if (this->selectedCameraIndex >= availableCameras.size()) {
        this->selectedCameraIndex = 0;
        this->selectedCameraName = availableCameras[0];
    }
    
    // Create the dropdown
    if (ImGui::BeginCombo("##Camera Selector", availableCameras[this->selectedCameraIndex].c_str())) {
        for (int i = 0; i < availableCameras.size(); i++) {
            bool isSelected = (this->selectedCameraIndex == i);
            if(isSelected && this->selectedCameraName==""){
                this->selectedCameraName = availableCameras[i];
            }
            if (ImGui::Selectable(availableCameras[i].c_str(), isSelected)) {
                this->selectedCameraIndex = i;
                this->selectedCameraName = availableCameras[i];
                // printf("selector");
            }
            
            // Set the initial focus when opening the combo
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}


void NodeArucoTracking::drawWorldSelector(){
    this->getValWorlds();
    if (this->availableWorlds.empty()) {
        ImGui::Text("No worlds avalible");
        return;
    }
    
    // Ensure selected index is valid
    if (this->selectedWorldIndex >= availableWorlds.size()) {
        this->selectedWorldIndex = 0;
        this->selectedWorld = availableWorlds[0];
    }
    
    // Create the dropdown
    if (ImGui::BeginCombo("##World Selector", availableWorlds[this->selectedWorldIndex].c_str())) {
        for (int i = 0; i < availableCameras.size(); i++) {
            bool isSelected = (this->selectedWorldIndex == i);
            if(isSelected && this->selectedWorld==""){
                this->selectedWorld = availableWorlds[i];
            }
            if (ImGui::Selectable(availableWorlds[i].c_str(), isSelected)) {
                this->selectedWorldIndex = i;
                this->selectedWorld = availableWorlds[i];
                // printf("selector");
            }
            
            // Set the initial focus when opening the combo
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
} //draws selector for selecting world frame

//void NodeArucoTracking::drawMainCamSelector(){}
std::shared_ptr<FrameRelation> NodeArucoTracking::calculateExtrinsicForParametars(std::string frameSrc, std::string frameDes) {

    if(frameSrc == frameDes){
        return cv::Mat::eye(4, 4, CV_64F);
    }
    auto relations = GlobalParams::getInstance().getCamRelations();
    //Direct relation exists
    for (auto& rel : relations) {
        if (rel->frame_src->frameNickName == frameSrc && 
            rel->frame_destination->frameNickName == frameDes) {
            return rel;
        }
    }
    //Inverted relation exists
    for (auto& rel : relations) {
        if (rel->frame_src->frameNickName == frameDes && 
            rel->frame_destination->frameNickName == frameSrc) {
            auto invertedRel = std::make_shared<FrameRelation>();
            invertedRel->frame_src = rel->frame_destination;
            invertedRel->frame_destination = rel->frame_src;
            invertedRel->distance_between_cams_in_cm = rel->distance_between_cams_in_cm;
            invertedRel->transformation_matrix = rel->transformation_matrix.inv();
            invertedRel->transformation_matrix_reprojection_error = rel->transformation_matrix_reprojection_error;
            GlobalParams::getInstance().addNewRelation(invertedRel);
            return invertedRel;
        }
    }
    //find a path using BFS
    std::unordered_map<std::string, std::shared_ptr<FrameRelation>> parent;
    std::queue<std::string> q;
    q.push(frameSrc);
    parent[frameSrc] = nullptr;

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();

        for (auto& rel : relations) {
            //check if relation is one i am looking for and check if there is already parent of destionation of that relation (already visited)
            if (rel->frame_src->frameNickName == current && 
                parent.find(rel->frame_destination->frameNickName) == parent.end()) {
                parent[rel->frame_destination->frameNickName] = rel;
                q.push(rel->frame_destination->frameNickName);
            }
            //same just inverted for inverted paths
            else if (rel->frame_destination->frameNickName == current && 
                     parent.find(rel->frame_src->frameNickName) == parent.end()) {
                auto invertedRel = std::make_shared<FrameRelation>();
                invertedRel->frame_src = rel->frame_destination;
                invertedRel->frame_destination = rel->frame_src;
                invertedRel->distance_between_cams_in_cm = rel->distance_between_cams_in_cm;
                invertedRel->transformation_matrix = rel->transformation_matrix.inv();
                invertedRel->transformation_matrix_reprojection_error = rel->transformation_matrix_reprojection_error;
                parent[rel->frame_src->frameNickName] = invertedRel;
                q.push(rel->frame_src->frameNickName);
            }
        }
    }
    //if on destionation parrent no path
    if (parent.find(frameDes) == parent.end()) {
        return nullptr; // No path found
    }
    //generating path back
    std::vector<std::shared_ptr<FrameRelation>> path;
    std::string current = frameDes;
    while (current != frameSrc) {
        path.push_back(parent[current]);
        current = parent[current]->frame_src->frameNickName;
    }
    std::reverse(path.begin(), path.end());
    // combine transformations
    cv::Mat T_total = cv::Mat::eye(4, 4, CV_64F);
    float maxreperror = 0.0;
    for (auto& rel : path) {
        if (maxreperror < rel->transformation_matrix_reprojection_error) {
            maxreperror = rel->transformation_matrix_reprojection_error;
        }
    
        // Clone the transformation to avoid modifying original
        cv::Mat T = rel->transformation_matrix.clone();
    
        // Scale only the translation part
        T.at<double>(0, 3) *= rel->distance_between_cams_in_cm;
        T.at<double>(1, 3) *= rel->distance_between_cams_in_cm;
        T.at<double>(2, 3) *= rel->distance_between_cams_in_cm;
    
        // Apply transformation
        T_total = T_total * T;
    }
    // Create the final relation
    auto newRel = std::make_shared<FrameRelation>();
    newRel->frame_src = GlobalParams::getInstance().getFrame(frameSrc);
    newRel->frame_destination = GlobalParams::getInstance().getFrame(frameDes);
    newRel->distance_between_cams_in_cm = cv::norm(T_total(cv::Rect(3, 0, 1, 3)));
    T_total.at<double>(0,3)/=newRel->distance_between_cams_in_cm;
    T_total.at<double>(1,3)/=newRel->distance_between_cams_in_cm;
    T_total.at<double>(2,3)/=newRel->distance_between_cams_in_cm;
    newRel->transformation_matrix = T_total;
    newRel->transformation_matrix_reprojection_error = maxreperror;
    GlobalParams::getInstance().addNewRelation(newRel);
    return newRel;
}
cv::Mat NodeArucoTracking::arucoPositions(cv::Mat img, std::string camframe, std::string worldFrame){
    auto relation = this->calculateExtrinsicForParametars(camframe, worldFrame);
    auto frames = GlobalParams::getInstance().getCamFrames();
    Structs::IntrinsicCamParams intrinsics;
    for (auto& cams : frames){
        if(cams->frameNickName == camframe){
            intrinsics = cams->intrinsicParams;
        }
    }
    if(relation==nullptr){
        std::string text = "No transform to world!";
        cv::Point position(this->resolution[0]/2, this->resolution[1]/2);
        int thickness = 2;
        cv::Scalar color(0, 0, 255);
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        cv::putText(img, text, position, fontFace, this->fontsize, color, thickness);
        return img;
    }
    auto transform = relation->transformation_matrix.clone();
    transform.at<double>(0,3)*=relation->distance_between_cams_in_cm;
    transform.at<double>(1,3)*=relation->distance_between_cams_in_cm;
    transform.at<double>(2,3)*=relation->distance_between_cams_in_cm;
    auto translation = transform(cv::Rect(3, 0, 1, 3));
    auto rotation = transform(cv::Rect(0, 0, 3, 3));

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    cv::aruco::ArucoDetector::detectMarkers(img, this->arucoDictionary, corners, ids);
    if(ids.empty()) return img; 

    for(size_t i = 0; i < ids.size(); i++) {
    // 3D object points for a marker (in its own coordinate system)
    std::vector<cv::Point3f> objectPoints = {
        cv::Point3f(-this->markerSize/200,  this->markerSize/200, 0), // Top-left
        cv::Point3f( this->markerSize/200,  this->markerSize/200, 0), // Top-right
        cv::Point3f( this->markerSize/200, -this->markerSize/200, 0), // Bottom-right
        cv::Point3f(-this->markerSize/200, -this->markerSize/200, 0)  // Bottom-left
    };
    std::vector<cv::Mat>poses;
    for(int i=0;i<ids.size();i++){
        cv::Vec3b rvec, tvec;
        cv::solvePnP(objectPoints, corners[i],
                    intrinsics.intrinsicMatrix,
                    intrinsics.distortionCoef,
                    rvec, tvec);
        cv::drawFrameAxes(img, 
            cam->intrinsicParams.intrinsicMatrix,
            cam->intrinsicParams.distortionCoef,
            rvec, 
            tvec, 
            markerSize/200);

        cv::Mat R;
        cv::Rodrigues(rvec, R);
        cv::Mat pose = cv::Mat::eye(4, 4, CV_64F);
        R.copyTo(pose(cv::Rect(0, 0, 3, 3)));
        pose.at<double>(0, 3) = tvec[0];
        pose.at<double>(1, 3) = tvec[1];
        pose.at<double>(2, 3) = tvec[2];
        pose = transform*pose; 
    }

    return img;
    
    
}// finds aruco marker on image and draws position and rotation on image

void sendData(std::string cam, long long int tstamp,  std::vector<cv::Point3f> rvecs, std::vector<cv::Point3f> tvecs){

}


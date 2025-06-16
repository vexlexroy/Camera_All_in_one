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
#include <nlohmann/json.hpp>

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
    ImGui::Text("Marker Size [cm]:");
    ImGui::SameLine();
    ImGui::DragFloat("##marker_size",&this->markerSize,0.05);
    ImGui::Text("world frame:");
    ImGui::SameLine();
    this->drawWorldSelector();
    ImGui::Text("view camera:");
    ImGui::SameLine();
    this->drawDropdownSelector();
    ImGui::Text("ip:");
    ImGui::SameLine();
    if(ImGui::InputTextWithHint("##ip", "0.0.0.0", this->ipBuffer, sizeof(this->ipBuffer)))
        this->ip=this->ipBuffer;
    ImGui::SameLine();
    ImGui::DragInt("port",&this->port);
    ImGui::SameLine();
    if(this->initialised) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); //green
    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)); //blue
    if(ImGui::Button("set")){
        try{
            auto ipaddress = boost::asio::ip::make_address(this->ip);
            this->conection = boost::asio::ip::udp::endpoint(
                ipaddress,  // IP address
                static_cast<unsigned short>(this->port)
            );
            std::cout << ipaddress << ":" << this->port << "\n";
            if (!socket) {
                socket = std::make_unique<boost::asio::ip::udp::socket>(asio_io);
            }
                this->socket->open(boost::asio::ip::udp::v4());
            this->initialised=true;
        }catch (const boost::system::system_error& e) {
            std::cerr << "Invalid address: " << e.what() << std::endl;
            this->initialised=false;
        }catch (const std::exception& e) {
            std::cerr << "Standard exception: " << e.what() << std::endl;
            this->initialised = false;
        }
    }
    ImGui::Text("sending:");
    ImGui::SameLine();
    ImGui::Checkbox("##sending", &this->sending);
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    // if(ImGui::Button("test calc")){
    //     this->calculateExtrinsicForParametars("laptop","laptop");
    // }

    //Tu update napraviti tako da se uzima freezed frame ako treba...
    Util::mat2Texture(this->lastMsg->first, this->lastMsg->second, this->texture); // if freezed -> update....

    //printf("texture id = %d\n", this->texture);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    //ImGui::ImageButton()
    if (ImGui::ImageButton("", (ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor))){
        ImGui::OpenPopup("preview");
    }
    ImGui::PopStyleColor();
    if (ImGui::BeginPopup("preview")){
        ImGui::SeparatorText("preview");
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

        ImGui::Text("show stats:");
        ImGui::SameLine();
        ImGui::Checkbox("##showstats",&this->showPositiontxt);
        ImGui::SameLine();
        ImGui::Text("font size:");
        ImGui::SameLine();
        ImGui::DragFloat("##font",&this->fontsize,0.5,0.1);
        ImGui::SameLine();
        ImGui::Text("show world:");
        ImGui::SameLine();
        ImGui::Checkbox("##showworld",&this->showWorld);

        
        ImGui::EndPopup();
    }

    

    this->mutex.unlock();
    return;
}


void NodeArucoTracking::recieve(std::shared_ptr<MessageBase> message, int connectorId) {
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);
    if(connector->connectorMessageType == Enums::MessageType::PICTURE) {
        auto msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        this->resolution[0] = msg->data->first.cols;
        this->resolution[1] = msg->data->first.rows;
        std::string camName = msg->camOrigin->frameNickName;

        this->mutex.lock();
        if(!this->isShiftPresed) {
            try {
                if(this->selectedWorld != "") {
                    std::vector<cv::Mat> poses;
                    std::vector<int> ids;
                    nlohmann::json markData = nullptr;
                    // Process the image
                    cv::Mat img = msg->data->first.clone();
                    cv::Mat processedImage = this->arucoPositions(img, camName, this->selectedWorld, poses, ids, markData);
                    if(this->sending && this->initialised)
                        this->sendData(camName, msg->getBaseTimestamp(), msg->startLagDuration, msg->getBaseDelay(), markData);
                    
                    // Create the pair correctly
                    if(msg->camOrigin->frameNickName == this->selectedCameraName) {
                        // if(this->showWorld){
                        //     processedImage=this->drawworldFrame(processedImage,camName,this->selectedWorld);
                        // }
                        this->lastMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(processedImage,msg->data->second.clone());
                    }
                    
                }
                else {
                    if(msg->camOrigin->frameNickName == this->selectedCameraName) {
                        this->lastMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(msg->data->first.clone(), msg->data->second.clone());
                    }
                }
            }
            catch (const cv::Exception& e) {
                std::cerr << "OpenCV Exception: " << e.what() << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Standard Exception: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknown Exception caught!" << std::endl;
            }
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
                   if(std::find(this->availableWorlds.begin(), this->availableWorlds.end(), frame->frameNickName) == this->availableWorlds.end()) 
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
    if (ImGui::BeginCombo("##World Selector", this->availableWorlds[this->selectedWorldIndex].c_str())) {
        for (int i = 0; i < this->availableWorlds.size(); i++) {
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
    

    if(frameSrc == frameDes){ // singular if its same
        auto singularRel = std::make_shared<FrameRelation>();
        singularRel->distance_between_cams_in_cm=1.0;
        singularRel->frame_destination = GlobalParams::getInstance().getFrame(frameDes);
        singularRel->frame_src = GlobalParams::getInstance().getFrame(frameSrc);
        singularRel->transformation_matrix=cv::Mat::eye(4, 4, CV_64F);
        singularRel->transformation_matrix_reprojection_error=0.0;
        return singularRel;
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
            invertedRel->transformation_matrix = rel->transformation_matrix.clone().inv();
            invertedRel->transformation_matrix_reprojection_error = rel->transformation_matrix_reprojection_error;
            // GlobalParams::getInstance().addNewRelation(invertedRel);
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
                invertedRel->transformation_matrix = rel->transformation_matrix.clone().inv();
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
        auto currentRel = parent[current];
        std::cout << "src: " << currentRel->frame_src->frameNickName 
                << " dest: " << currentRel->frame_destination->frameNickName << "\n";
        std::cout << "rel: " << currentRel->transformation_matrix << "\n";
        std::cout << "dist: " << currentRel->distance_between_cams_in_cm << "\n" <<"\n";
        current = currentRel->frame_src->frameNickName;
        path.push_back(currentRel);
    }
    // std::reverse(path.begin(), path.end());
    // combine transformations
    cv::Mat T_total = cv::Mat::eye(4, 4, CV_64F);
    float maxreperror = 0.0;
    std::cout << "\n";
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
        std::cout <<rel->frame_destination->frameNickName <<"_"<<rel->frame_src->frameNickName  << "*";
        T_total = T_total.clone() * T.clone();
    }
    std::cout << "\n";
    // Create the final relation
    auto newRel = std::make_shared<FrameRelation>();
    newRel->frame_src = GlobalParams::getInstance().getFrame(frameSrc);
    newRel->frame_destination = GlobalParams::getInstance().getFrame(frameDes);
    newRel->distance_between_cams_in_cm = cv::norm(T_total.clone()(cv::Rect(3, 0, 1, 3)));
    T_total.at<double>(0,3)/=(newRel->distance_between_cams_in_cm);
    T_total.at<double>(1,3)/=(newRel->distance_between_cams_in_cm);
    T_total.at<double>(2,3)/=(newRel->distance_between_cams_in_cm);
    newRel->transformation_matrix = T_total.clone();
    newRel->transformation_matrix_reprojection_error = maxreperror;
    GlobalParams::getInstance().addNewRelation(newRel);
    return newRel;
}
cv::Mat NodeArucoTracking::arucoPositions(cv::Mat img, std::string camframe, std::string worldFrame, std::vector<cv::Mat>& allposes, std::vector<int>& allids, nlohmann::json& jsonData){
    // printf("detecting A\n");
    auto relation = this->calculateExtrinsicForParametars(camframe, worldFrame);
    // printf("relation calc\n");
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
    cv::aruco::ArucoDetector detect;
    detect.setDictionary(this->arucoDictionary);
    detect.detectMarkers(img, corners, ids);

    cv::Mat revTransform = transform.clone().inv();
    cv::Mat tvecT = revTransform(cv::Rect(3, 0, 1, 3)).clone();
    cv::Mat rT = revTransform(cv::Rect(0, 0, 3, 3)).clone();
    
    if(this->showWorld){
        // cv::Mat rvecT;
        // cv::Rodrigues(rT, rvecT); 
        // cv::drawFrameAxes(img, intrinsics.intrinsicMatrix, intrinsics.distortionCoef, rvecT, tvecT, markerSize*2, 1);
        std::vector<cv::Point3f> world_axes_pts = {
        cv::Point3f(0, 0, 0),                      // origin
        cv::Point3f(markerSize*2, 0, 0),                 // X axis
        cv::Point3f(0, markerSize*2, 0),                 // Y axis
        cv::Point3f(0, 0, markerSize*2)                  // Z axis
        };
        std::vector<cv::Point3f> cam_axes_pts;
        for (auto& pt : world_axes_pts) {
            cv::Mat pt_mat = (cv::Mat_<double>(3,1) << pt.x, pt.y, pt.z);
            cv::Mat pt_cam = rT * pt_mat + tvecT;
            cam_axes_pts.emplace_back(pt_cam.at<double>(0), pt_cam.at<double>(1), pt_cam.at<double>(2));
        }
        std::vector<cv::Point2f> image_pts;
        cv::projectPoints(cam_axes_pts, cv::Vec3d(0,0,0), cv::Vec3d(0,0,0),
                        intrinsics.intrinsicMatrix, intrinsics.distortionCoef,
                        image_pts);
        cv::line(img, image_pts[0], image_pts[1], cv::Scalar(0,0,255), 1); // X - red
        cv::line(img, image_pts[0], image_pts[2], cv::Scalar(0,255,0), 1); // Y - green
        cv::line(img, image_pts[0], image_pts[3], cv::Scalar(255,0,0), 1); // Z - blue
    }
    

    if(ids.empty()) return img; 
    
    std::vector<cv::Point3f> objectPoints = {
        cv::Point3f(-markerSize/2.0f, markerSize/2.0f, 0),
        cv::Point3f( markerSize/2.0f, markerSize/2.0f, 0), 
        cv::Point3f( markerSize/2.0f, -markerSize/2.0f, 0), 
        cv::Point3f(-markerSize/2.0f,  -markerSize/2.0f, 0)  
    };

    std::vector<cv::Mat>poses;
    nlohmann::json dataArray = nlohmann::json::array();
    cv::Mat zeroDistCoeffs = cv::Mat::zeros(intrinsics.distortionCoef.size(), intrinsics.distortionCoef.type());
    for(int i=0;i<ids.size();i++){
        cv::Vec3d rvec, tvec;
        cv::solvePnP(objectPoints, corners[i],
                    intrinsics.intrinsicMatrix,
                    zeroDistCoeffs,
                    rvec, tvec);
        cv::drawFrameAxes(img, intrinsics.intrinsicMatrix, intrinsics.distortionCoef, rvec, tvec, markerSize/2.0);
        

        cv::Mat R;
        cv::Rodrigues(rvec, R);
        cv::Mat pose = cv::Mat::eye(4, 4, CV_64F);
        R.copyTo(pose(cv::Rect(0, 0, 3, 3)));
        pose.at<double>(0, 3) = tvec[0];
        pose.at<double>(1, 3) = tvec[1];
        pose.at<double>(2, 3) = tvec[2];


        cv::Mat tpose = transform*pose;

        poses.push_back(tpose);
        // draw part
        int id = ids[i];
        float x = tpose.at<double>(0,3);
        float y = tpose.at<double>(1,3);
        float z = tpose.at<double>(2,3);
        double yaw,roll,pitch;
        double qw,qx,qy,qz;
        this->rotationMatrixToEulerAngles(tpose(cv::Rect(0, 0, 3, 3)),roll,pitch,yaw);
        this->rotationMatrixToQuaternion(tpose(cv::Rect(0, 0, 3, 3)), qw,qx,qy,qz);
        if(this->showPositiontxt){
            std::stringstream ss_x, ss_y, ss_z, ss_roll, ss_pitch, ss_yaw;
            ss_x << "x: " << std::fixed << std::setprecision(1) << x << " ";
            ss_y << "y: " << std::fixed << std::setprecision(1) << y << " ";
            ss_z << "z: " << std::fixed << std::setprecision(1) << z << "cm";
            ss_roll << "r: "<< std::fixed << std::setprecision(1) << roll;
            ss_pitch << "p: "<< std::fixed << std::setprecision(1) << pitch;
            ss_yaw << "y: "<< std::fixed << std::setprecision(1) << yaw;
            cv::Point position(corners[i][0].x, corners[i][0].y);
            int thickness = 2;
            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            int baseline = 0;
            cv::Size textSize = cv::getTextSize("Test", fontFace, this->fontsize, thickness, &baseline);
            int lineHeight = textSize.height + 5;
            int x_offset = 0;
            drawColoredSegment(img, ss_x.str(), cv::Scalar(0, 0, 255), position, this->fontsize, thickness, &x_offset);  // Red
            drawColoredSegment(img, ss_y.str(), cv::Scalar(0, 255, 0), position, this->fontsize, thickness, &x_offset);  // Green
            drawColoredSegment(img, ss_z.str(), cv::Scalar(255, 0, 0), position, this->fontsize, thickness, &x_offset);   // Blue
            x_offset = 0;
            cv::Point rotationPosition(position.x, position.y + lineHeight);
            drawColoredSegment(img, ss_roll.str(), cv::Scalar(0, 0, 255), rotationPosition, this->fontsize, thickness, &x_offset);  // Red
            drawColoredSegment(img, ss_pitch.str(), cv::Scalar(0, 255, 0), rotationPosition, this->fontsize, thickness, &x_offset);  // Green
            drawColoredSegment(img, ss_yaw.str(), cv::Scalar(255, 0, 0), rotationPosition, this->fontsize, thickness, &x_offset);     // Blue
        }
        nlohmann::json markerData;
        markerData["id"] = id;
        markerData["x"] = x/100.0f;
        markerData["y"] = y/100.0f;
        markerData["z"] = z/100.0f;
        markerData["yaw"] = yaw;
        markerData["roll"] = roll;
        markerData["pitch"] = pitch;
        markerData["qw"] = qw;
        markerData["qx"] = qx;
        markerData["qy"] = qy;
        markerData["qz"] = qz;
        dataArray.push_back(markerData);
    }
    
    // cameraData["time"] = 
    allids=ids;
    allposes=poses;
    jsonData=dataArray;
    return img;
    
    
}// finds aruco marker on image and draws position and rotation on image

void NodeArucoTracking::sendData(std::string cam, long long int tstamp, long long int delay, long long int delay2,  nlohmann::json markerData){
    nlohmann::json frameData;
    frameData["cam"] = cam;
    frameData["time"] = tstamp;
    frameData["cam_delay"] = delay;
    frameData["processing_delay"] = (delay2-delay*1000);
    frameData["data"] = markerData;
    if(!markerData.empty()){
        // std::cout << frameData.dump() << "\n";
        // printf("send:\n");
        this->socket->send_to(boost::asio::buffer(frameData.dump()),this->conection);
    }
    return;
}


void NodeArucoTracking::rotationMatrixToEulerAngles(const cv::Mat& R, double& roll, double& pitch, double& yaw) {
    // Using YXZ convention (common in computer vision)
    pitch = asin(-R.at<double>(2, 0));
    const double cos_pitch = cos(pitch);
    roll = atan2(R.at<double>(2, 1)/cos_pitch, R.at<double>(2, 2)/cos_pitch);
    yaw = atan2(R.at<double>(1, 0)/cos_pitch, R.at<double>(0, 0)/cos_pitch);
    // Convert to degrees
    roll *= 180 / CV_PI;
    pitch *= 180 / CV_PI;
    yaw *= 180 / CV_PI;
}

void NodeArucoTracking::rotationMatrixToQuaternion(const cv::Mat& R, double& qw,double& qx,double& qy,double& qz) {
    // Ensure R is a 3x3 float/double matrix
    CV_Assert(R.rows == 3 && R.cols == 3 && (R.type() == CV_64F || R.type() == CV_32F));
    double m00 = R.at<double>(0,0), m01 = R.at<double>(0,1), m02 = R.at<double>(0,2);
    double m10 = R.at<double>(1,0), m11 = R.at<double>(1,1), m12 = R.at<double>(1,2);
    double m20 = R.at<double>(2,0), m21 = R.at<double>(2,1), m22 = R.at<double>(2,2);
    double trace = m00 + m11 + m22;
    if (trace > 0) {
        double S = sqrt(trace + 1.0) * 2.0; // S = 4*qw
        qw = 0.25 * S;
        qx = (m21 - m12) / S;
        qy = (m02 - m20) / S;
        qz = (m10 - m01) / S;
    } else if ((m00 > m11) && (m00 > m22)) {
        double S = sqrt(1.0 + m00 - m11 - m22) * 2.0; // S = 4*qx
        qw = (m21 - m12) / S;
        qx = 0.25 * S;
        qy = (m01 + m10) / S;
        qz = (m02 + m20) / S;
    } else if (m11 > m22) {
        double S = sqrt(1.0 + m11 - m00 - m22) * 2.0; // S = 4*qy
        qw = (m02 - m20) / S;
        qx = (m01 + m10) / S;
        qy = 0.25 * S;
        qz = (m12 + m21) / S;
    } else {
        double S = sqrt(1.0 + m22 - m00 - m11) * 2.0; // S = 4*qz
        qw = (m10 - m01) / S;
        qx = (m02 + m20) / S;
        qy = (m12 + m21) / S;
        qz = 0.25 * S;
    }
    return;
}

void NodeArucoTracking::drawColoredSegment(cv::Mat& img, const std::string& text, const cv::Scalar& color, cv::Point& position, double fontsize, int thickness, int* x_offset) {
    int baseline = 0;
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    cv::Size textSize = cv::getTextSize(text, fontFace, fontsize, thickness, &baseline);

    cv::putText(img, text, 
    cv::Point(position.x + *x_offset, position.y), 
    fontFace, fontsize, color, thickness);

    *x_offset += textSize.width;
}


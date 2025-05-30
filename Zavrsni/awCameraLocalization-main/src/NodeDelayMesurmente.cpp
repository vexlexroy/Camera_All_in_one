#include "NodeDelayMesurmente.hpp"
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
#include <boost/asio.hpp>

//this->lastMsg->first  je slika dobivena u poruci od drugog nodea


NodeDelayMesurmente::NodeDelayMesurmente(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEDELAYMESURMENTE;
}

std::shared_ptr<NodeBase> NodeDelayMesurmente::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeDelayMesurmente>(uniqueId);
}



std::vector<Enums::MessageType> NodeDelayMesurmente::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);
    inMessageTypes.push_back(Enums::MessageType::INT);
    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeDelayMesurmente::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::INT);
    outMessageTypes.push_back(Enums::MessageType::PICTURE);
    return outMessageTypes;
}


std::string NodeDelayMesurmente::getDescription(){
    return "Node za mijerenje kasnjenja kamere";
}

std::string NodeDelayMesurmente::getName(){
    return "Node-delay-mesurmente";
}

Enums::NodeType NodeDelayMesurmente::getType(){
    return Enums::NodeType::NODEDELAYMESURMENTE;
}

void NodeDelayMesurmente::drawNodeParams(){
    
    
    ImGui::PushItemWidth(190);
    ImGui::BeginGroup();
    ImGui::Text("Serial Port:");
    ImGui::SameLine();
    ImGui::InputText("##serial_port", this->port, IM_ARRAYSIZE(this->port));
    ImGui::Text("Baudrate:");
    ImGui::SameLine();
    ImGui::DragInt("##brate", &this->baudRate, 1.0f, 0, 1000000);
    ImGui::Checkbox("Syncronised testing mode", &this->syncedTest);
    if(!this->syncedTest){
        ImGui::Text("Test period:");
        ImGui::SameLine();
        ImGui::DragInt("##period", &this->test_period, 1.0f, 0, 1000000);
        ImGui::Checkbox("Start Periodic Testing", &this->isPeriodicTest);
        if (ImGui::Button("Set Port")) {
            this->disconnect();
            this->using_port=std::string(port);
            this->connect(this->using_port, this->baudRate);
        }
        if (ImGui::Button("TEST")) {
            this->startTest=true;
            this->lineTime=Util::timeSinceEpochMicroseconds();
        } 
    }else{
        ImGui::Checkbox("Initiator", &this->initiator);
        if(this->initiator){
            ImGui::Text("Test period:");
            ImGui::SameLine();
            ImGui::DragInt("##period", &this->test_period, 1.0f, 0, 1000000);
            ImGui::Checkbox("Start Periodic Testing", &this->isPeriodicTest);
        }
    }
    if (this->isConnected()){
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("Connected");
        ImGui::PopStyleColor();
    }else{
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::Text("Not Connected");
        ImGui::PopStyleColor();
    }
   // ImGui::Checkbox("Use average of 3 tests",&this->useAvg); //TODO currently not working ads functionality to get average of multiple mesurmantes
    if (this->startTest){
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("Testing...");
        ImGui::PopStyleColor();
    }
    else{
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        if(this->useAvg) ImGui::Text("Last Average Delay: %d",this->lastAvgDelay);
        else ImGui::Text("Last Delay: %d",this->lastDelay);
        ImGui::PopStyleColor();
    }
    ImGui::EndGroup();
   
}

void NodeDelayMesurmente::drawNodeWork(){
    this->mutex.lock();
    if(this->lastMsg == nullptr){
        this->mutex.unlock();
        return;
    }
    
            
    //Util::matToTexture(this->frame, &this->texture, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    cv::Mat hsvFrame; //SLIKA
    cvtColor(this->lastMsg->first, hsvFrame, cv::COLOR_BGR2HSV);// SLIKA KOJU DOBIJE U PORUCI

    for(int x = 0; x < hsvFrame.cols; x++){ // loop that gets all pixels hsv values and turnes all matching pixels white and non matching gray
        for(int y = 0; y < hsvFrame.rows; y++){
            //Ako je filter value bio 0 tada postaje 50. Ako je filter value bio 255 i nije prošao provjeru tada postaje 100, inače 255
            if(this->lastMsg->second.at<u_char>(y,x) == 0){
                continue;
            }
            if(this->roiOn && // ROI is ON?
                !(x>=this->roix[0] && y>=this->roiy[0]&& //if its outside ROI range
                 x<=this->roix[1] && y<=this->roiy[1])){
                    this->lastMsg->second.at<u_char>(y,x) = 40;
                    continue;
                }

            cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(y,x); //hsv value of pixel at x,y

            //HUE
            if(hsv[0] >= this->hue[0] && hsv[0] <= this->hue[1]
            && hsv[1] >= this->sat[0] && hsv[1] <= this->sat[1]
            && hsv[2] >= this->val[0] && hsv[2] <= this->val[1]){// filtrirana slika se sprema u this->lastMsg->second
                this->lastMsg->second.at<u_char>(y,x) = 255;
            }
            else{
                this->lastMsg->second.at<u_char>(y,x) = 70;
            }
        }
    }

    //BLOB FINDING
    if(this->showBlob){
        cv::Mat gray;
        if (this->lastMsg->second.channels() > 1) {
            cv::cvtColor(this->lastMsg->second, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = this->lastMsg->second.clone();
        }
        // Ensure binary image
        cv::Mat binary;
        cv::threshold(gray, binary, 100, 255, cv::THRESH_BINARY);
        // Labels, stats, and centroids
        cv::Mat labels, stats, centroids;
        int numLabels = cv::connectedComponentsWithStats(binary, labels, stats, centroids);
        for (int i = 1; i < numLabels; ++i) {
            int area = stats.at<int>(i, cv::CC_STAT_AREA);
            if (area >= this->area[0] && area <= this->area[1]) {
                double centerx = centroids.at<double>(i, 0);
                double centery = centroids.at<double>(i, 1);
                cv::circle(this->lastMsg->second, cv::Point((int)centerx, (int)centery), (int)std::sqrt(this->area[1]/3.14), cv::Scalar(0, 255, 0), 2);
                cv::circle(this->lastMsg->second, cv::Point((int)centerx, (int)centery), (int)std::sqrt(this->area[0]/3.14), cv::Scalar(0, 0, 255), 3);
            }
        }
    }

    // Turn on and off LED for visual confirmation and filtering
    if(this->ledOn && !this->isLedOn){
        this->sendData("LEDTESTON");
        this->isLedOn = true;
    }
    if(!this->ledOn && this->isLedOn){
        this->sendData("LEDTESTOFF");
        this->isLedOn = false;
    }

    //Tu update napraviti tako da se uzima freezed frame ako treba...
    Util::mat2Texture(this->lastMsg->first, this->lastMsg->second, this->texture); // if freezed -> update....

    
    //printf("texture id = %d\n", this->texture);
    

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    //ImGui::ImageButton()
    if (ImGui::ImageButton("", (ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor))){

        ImGui::OpenPopup("choose ROI");
        //Make static frame
        
        
        //Util::mat2Texture(this->hsvColorChooserFrame, this->hsvColorChooserTexture);
    }
    ImGui::PopStyleColor();
    if (ImGui::BeginPopup("choose ROI")){
        ImGui::SeparatorText("choose ROI");
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
        ImGui::SameLine();
        //Now create HSV
        float hsvValues[3][255] = {0};
        
        for (int i = 0; i<hsvFrame.rows; i++)
        {
            for (int j = 0; j<hsvFrame.cols; j++)
            {
                cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(i,j);
                hsvValues[0][hsv[0]]++;
                hsvValues[1][hsv[1]]++;
                hsvValues[2][hsv[2]]++;
            }
        }

        int hueMaxAppeared = 0;
        int satMaxAppeared = 0;
        int valMaxAppeared = 0;
        for(int i = 0; i < 255; i++){
            if(hsvValues[0][i] > hueMaxAppeared){
                hueMaxAppeared = hsvValues[0][i];
            }
            if(hsvValues[1][i] > satMaxAppeared){
                satMaxAppeared = hsvValues[1][i];
            }
            if(hsvValues[2][i] > valMaxAppeared){
                valMaxAppeared = hsvValues[2][i];
            }
        }
        
        //Normalize values between 0 and 1
        for(int i = 0; i < 255; i++){
            hsvValues[0][i] = hsvValues[0][i]/hueMaxAppeared;
            hsvValues[1][i] = hsvValues[1][i]/satMaxAppeared;
            hsvValues[2][i] = hsvValues[2][i]/valMaxAppeared;
        }

        //ImGui::DragInt("Average")
        //postaviti min i max na negativne vrijednosti, tako da se vidi 
        hsvValues[0][this->hue[0]] = -0.2;
        hsvValues[0][this->hue[1]] = -0.2;

        hsvValues[1][this->sat[0]] = -0.2;
        hsvValues[1][this->sat[1]] = -0.2;

        hsvValues[2][this->val[0]] = -0.2;
        hsvValues[2][this->val[1]] = -0.2;

        ImGui::SameLine();
        ImGui::BeginGroup();
        if(ImGui::Button("reset") || ImGui::IsKeyPressed(ImGuiKey_R)){
            this->hue[0] = 255;
            this->hue[1] = 0;
            this->sat[0] = 255;
            this->sat[1] = 0;
            this->val[0] = 255;
            this->val[1] = 0;
            this->roix[0] = 0; this->roix[1] = this->resolution[0];
            this->roiy[0] = 0; this->roiy[1] = this->resolution[1];
            // ništa nesmije prolaziti...
        }
        ImGui::SeparatorText("HUE");
        ImGui::PlotLines("hueLine", hsvValues[0], 255, 0, 0, -0.2, 1, ImVec2(300, 80));
        ImGui::DragInt2("hue", this->hue, 1, 0, 255);
        
        ImGui::SeparatorText("SEPARATION");
        ImGui::PlotLines("sepLine", hsvValues[1], 255, 0, 0, -0.2, 1, ImVec2(300, 80));
        ImGui::DragInt2("sat", this->sat, 1, 0, 255);
        ImGui::SeparatorText("VALUE");
        ImGui::PlotLines("valLine", hsvValues[2], 255, 0, 0, -0.2, 1, ImVec2(300, 80));
        ImGui::DragInt2("val", this->val, 1, 0, 255);
        ImGui::SeparatorText("ROI");
        ImGui::DragInt2("x,x'", this->roix, 1, 0, this->resolution[0]);
        ImGui::DragInt2("y,y'", this->roiy, 1, 0, this->resolution[1]);
        ImGui::SeparatorText("ROI ON");
        ImGui::Checkbox("roi",&this->roiOn);
        ImGui::SeparatorText("LED ON");
        ImGui::Checkbox("led",&this->ledOn);
        ImGui::SeparatorText("BLOB SIZE");
        ImGui::DragInt2("min,max'", this->area, 1, 0, 10000);
        ImGui::Checkbox("show blobs",&this->showBlob);
        ImGui::EndGroup();
        //Add picture drag n drop
        //ImGui::Text("%.2f, %.2f\n", pos.x, pos.y);
        //ImGui::Text("%.2f, %.2f\n", io.MousePos.x, io.MousePos.y);
        //ImGui::Text("%.2f, %.2f\n", io.MousePos.x - pos.x, io.MousePos.y - pos.y);
        //ImGui::Text("%.2f, %.2f\n", zoom_T.x, zoom_T.y);
        //if shiftReleased -> update with new frame
        //if shiftHolded -> get saved frame
        if(ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
            ImVec2 dragMousePos = ImGui::GetMousePos();
            int dragXPicturePos = zoom_T.x * this->resolution[0];
            int dragYPicturePos = zoom_T.y * this->resolution[1];

            if(dragXPicturePos < this->resolution[0] && dragXPicturePos > 0
            && dragYPicturePos < this->resolution[1] && dragYPicturePos > 0){
                //extract that pixel
                cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(dragYPicturePos, dragXPicturePos);
                //update hsv range
                if(hsv[0] < this->hue[0]){
                    this->hue[0] = hsv[0];
                }
                if(hsv[0] > this->hue[1]){
                    this->hue[1] = hsv[0];
                }

                if(hsv[1] < this->sat[0]){
                    this->sat[0] = hsv[1];
                }
                if(hsv[1] > this->sat[1]){
                    this->sat[1] = hsv[1];
                }

                if(hsv[2] < this->val[0]){
                    this->val[0] = hsv[2];
                }
                if(hsv[2] > this->val[1]){
                    this->val[1] = hsv[2];
                }
            }
        }
        if(ImGui::IsMouseDragging(ImGuiKey_R) || ImGui::IsMouseClicked(ImGuiKey_R)){
            ImVec2 dragMousePos = ImGui::GetMousePos();
            int dragXPicturePos = zoom_T.x * this->resolution[0];
            int dragYPicturePos = zoom_T.y * this->resolution[1];

            if(dragXPicturePos < this->resolution[0] && dragXPicturePos > 0
            && dragYPicturePos < this->resolution[1] && dragYPicturePos > 0){
                
               
            }
        }
        ImGui::EndPopup();
    }
    this->mutex.unlock();
    return;
}



void NodeDelayMesurmente::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if(connector->connectorMessageType == Enums::MessageType::INT){
        if(this->syncedTest && !this->initiator) {this->startTest = true; this->freetotest=true;}
        if(this->syncedTest && this->initiator) this->freetotest = true;

    }

    this->LastMessage = message;

    if(connector->connectorMessageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        
        
        this->mutex.lock();
        if(this->startTest && !this->isConnected()){
            this->connect(this->port,this->baudRate);
        }
        this->mutex.unlock();
        
        
        //printf("Message came\n");
        this->mutex.lock();
        this->resolution[0]=msg->data->first.cols;
        this->resolution[1]=msg->data->first.rows;
        
        /*
        if(this->freezeFrameFlag)
        {
            this->freezeFrameFlag = false;
            this->freezeMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));
        }
        */
        if(!this->isShiftPresed)
        {
            this->lastMsg = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone())); // nije međudretveno sigurno
            // cv::imshow("Original Frame", msg->data->first); // This shows the original frame
            //cv::waitKey(1); // Use a small delay to allow OpenCV to update the windo

        }

        //create new filtered msg here based on set hue, saturation and value, change just filter part...
        //just update second part of filter, based on set hue sat and val.

        cv::Mat hsvFrame;
        cvtColor(msg->data->first, hsvFrame, cv::COLOR_BGR2HSV);

        for(int x = 0; x < hsvFrame.cols; x++){ // aplys filter to message
            for(int y = 0; y < hsvFrame.rows; y++){
                //Ako je filter value bio 0 tada postaje 50. Ako je filter value bio 255 i nije prošao provjeru tada postaje 100, inače 255
                if(msg->data->second.at<u_char>(y,x) == 0){
                    continue;
                }

                if(this->roiOn // ROI is ON?
                && !(x>=this->roix[0] && y>=this->roiy[0] && x<=this->roix[1] && y<=this->roiy[1])){
                    msg->data->second.at<u_char>(y,x) = 0;
                    continue;
                }

                cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(y,x);

                //HUE
                if(hsv[0] >= this->hue[0] && hsv[0] <= this->hue[1]
                && hsv[1] >= this->sat[0] && hsv[1] <= this->sat[1]
                && hsv[2] >= this->val[0] && hsv[2] <= this->val[1]){
                    msg->data->second.at<u_char>(y,x) = 255;
                }
                else{
                    msg->data->second.at<u_char>(y,x) = 0;
                }
                
            }
        }
        // cv::imshow("blobs",msg->data->second);
        // cv::waitKey(1);
        if(this->startTest && this->freetotest){
            if(this->isConnected()){
                this->initiateTest(msg->data->second);
                this->test_timer=NULL;
            }
        }
        else if (this->isPeriodicTest){
            if (this->test_timer==NULL) this->test_timer = Util::timeSinceEpochMicroseconds();
            else{
                long long int cTime=Util::timeSinceEpochMicroseconds();
                int timeSinceLastT = (cTime - this->test_timer)/1000000; //sec
                if(timeSinceLastT>=this->test_period){
                    this->lineTime=Util::timeSinceEpochMicroseconds();
                    this->startTest=true;
                }
            }
        }
        // if(ImGui::IsKeyPressed(ImGuiKey_E)){
        //     cv::imshow("filtered", msg->data->second);
        // }

        /*
        for(int i = 0; i < this->connectors.size(); i++){

            if(connectors[i]->connectorMessageType == Enums::MessageType::PICTURE){
                //make copy of message, and send to all connectors...
                std::shared_ptr<std::pair<cv::Mat, cv::Mat>> clonedData = std::make_shared<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));

                std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msgCpy = std::make_shared<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(Enums::MessageType::PICTURE , msg);
                msgCpy->data = clonedData;
                this->send(msgCpy, connectors[i]);
            }
        }
        */

        this->sendAll(msg); // will send original and edited image to all ports
        this->mutex.unlock();
           
    }    
    
}

int NodeDelayMesurmente::blobCount(cv::Mat image) {
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    // Ensure binary image
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY); //doesent matter here its black and white
    // Labels, stats, and centroids
    cv::Mat labels, stats, centroids;
    int numLabels = cv::connectedComponentsWithStats(binary, labels, stats, centroids);
    int count = 0;
    for (int i = 1; i < numLabels; ++i) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area >= this->area[0] && area <= this->area[1]) {
            count++;
        }
    }

    return count;
}

bool NodeDelayMesurmente::testDelay(cv::Mat image){
    if(!this->isTesting){// proverava jeli vec u testiranju
        this->currentTime = Util::timeSinceEpochMicroseconds();//std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        this->isTesting=true;
    }
    else{
        long long int time = Util::timeSinceEpochMicroseconds();//std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        int delay = static_cast<int>(time - this->currentTime)/1000;
        int count = blobCount(image);
        // std::cout << "StartBlob: " << this->lastBlobCount << "CurrentBlob: " << count << "delay: " << delay << "\n";
        if(delay> this->maxTime){
            this->isTesting=false;
            this->startTest=false;
            printf("Time limit (No Detection in %d ms)!!!", this->maxTime);
            // this->disconnect();
            this->lastDelay=-1;
            return true;
        }
        else
        if(count>this->lastBlobCount){
            this->isTesting=false;
            // this->startTest=false;
            this->lastDelay=delay;
            // this->ledOn=false;
            return true;
        }
    }
    return false;

}

void NodeDelayMesurmente::initiateTest(cv::Mat image){
    if(this->sentRequest){
        if(this->response){
            bool test = this->testDelay(image);
            if(test && this->useAvg){ //will not happen wasnt implemented. always false
                this->last_Delays[this->numOfftests]=this->lastDelay;
                this->numOfftests++;
                if(this->numOfftests<sizeof(this->last_Delays)/sizeof(last_Delays[0])){
                    this->sentRequest = false;
                    // this->startTest = false;
                    this->response = false;
                    this->ledOn=false;
                    return;

                }
                else{
                    int sum=0;
                    for (int i=0;i<numOfftests;++i){
                        sum=sum+this->last_Delays[i];
                    }
                    this->lastAvgDelay=sum/numOfftests;
                    this->numOfftests=0;
                }
            }else if(test){ //send delay to connector TODO
                std::shared_ptr<Message<int>> delayMsg = std::make_shared<Message<int>>(Enums::MessageType::INT,std::static_pointer_cast<MessageBase>(this->LastMessage));
                delayMsg->data = this->lastDelay;
                for(auto& connector : this->connectors) {
                    if(connector->connectorMessageType == Enums::MessageType::INT) {
                        // printf("Found INT connector, sending... %d\n",delayMsg->data);
                        this->send(delayMsg, connector);
                    }
                }  
            }
            if(!this->isTesting){
                this->sentRequest = false;
                this->startTest = false;
                this->response = false;
                this->ledOn=false;
                this->disconnect();
                if(this->syncedTest){
                    this->freetotest = false;
                }
                // Util::delay(20);
            }
        }
        else{
            if(this->readData()){
                this->response=true;
            }
            else{
                long long int ctime=Util::timeSinceEpochMicroseconds();
                if((ctime-this->startTime)/1000>this->timeOut){
                    this->sentRequest = false;
                    this->startTest = false;
                    this->response = false;
                    printf("Timed Out (No Response in %d ms)!!!",this->timeOut);
                    this->disconnect();
                    if(this->syncedTest){
                        this->freetotest = false;
                    }
                }
            }
        }
    }
    else{
        this->startTime=Util::timeSinceEpochMicroseconds();
        this->lastBlobCount=blobCount(image);
        // this->sendData("LEDTESTOFF\n");
        this->sendData("TEST\n");
        this->isLedOn=true;
        this->sentRequest=true;
        //this->showBlob=false;
    }

}

bool NodeDelayMesurmente::connect(std::string port, unsigned int baud) {
    if (this->serial && this->serial->is_open()) {
        std::cout << "Already connected to a serial port." << std::endl;
        return false;
    }

    try {
        this->serial = std::make_unique<boost::asio::serial_port>(this->io, port);
        this->serial->set_option(boost::asio::serial_port_base::baud_rate(baud));
        std::cout << "Connected to " << port << " at " << baud << " baud." << std::endl;
        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error connecting to serial port: " << e.what() << std::endl;
        this->serial.reset();
        // Util::delay(150);
        return false;
    }
}

void NodeDelayMesurmente::disconnect() {
    if (this->serial && this->serial->is_open()) {
        this->serial->close();
        std::cout << "Serial port disconnected." << std::endl;
    }
    this->serial.reset();
}

bool NodeDelayMesurmente::isConnected(){
    return this->serial && this->serial->is_open();
}

bool NodeDelayMesurmente::isPortAvailable(std::string port) {
    try {
        boost::asio::io_context io;
        boost::asio::serial_port test_port(io);
        test_port.open(port);
        return false; // If open succeeds, port is in use
    } catch (...) {
        return true; // Port is available
    }
}

void NodeDelayMesurmente::sendData(std::string data) {
    if (isConnected()) {
        boost::asio::write(*this->serial, boost::asio::buffer(data));
    }
}

bool NodeDelayMesurmente::readData() {
    //printf("in read serial.\n");
    if (isConnected()) {
        //printf("passed conected\n");
        boost::asio::streambuf buf;
        boost::system::error_code ec;
        // Read until newline or your custom terminator
        boost::asio::read_until(*this->serial, buf, '\n', ec);
        printf("pass read\n");
        if (!ec) {
            std::istream is(&buf);
            std::string receivedmsg;
            std::getline(is, receivedmsg);
            std::cout << "REC msg: " << receivedmsg << "\n";

            // Trim any carriage return if needed
            if (!receivedmsg.empty() && receivedmsg.back() == '\r') {
                receivedmsg.pop_back();
            }
            if (receivedmsg == "LEDON") {
                return true;
            }
        }
    }
    return false;
}


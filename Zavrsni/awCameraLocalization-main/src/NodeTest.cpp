#include "NodeTest.hpp"
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

NodeTest::NodeTest(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODETEST;
}

std::shared_ptr<NodeBase> NodeTest::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeTest>(uniqueId);
}



std::vector<Enums::MessageType> NodeTest::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeTest::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::PICTURE);
    
    return outMessageTypes;
}


std::string NodeTest::getDescription(){
    return "Ovo je testni node koji nema nikakvu drugu svrhu";
}

std::string NodeTest::getName(){
    return "Node-color-treshold";
}

Enums::NodeType NodeTest::getType(){
    return Enums::NodeType::NODETEST;
}

void NodeTest::drawNodeParams(){
    
   
}

void NodeTest::drawNodeWork(){
    this->mutex.lock();
    if(this->lastMsg == nullptr){
        this->mutex.unlock();
        return;
    }
    
            
    //Util::matToTexture(this->frame, &this->texture, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    cv::Mat hsvFrame;
    cvtColor(this->lastMsg->first, hsvFrame, cv::COLOR_BGR2HSV);

    for(int x = 0; x < hsvFrame.cols; x++){
        for(int y = 0; y < hsvFrame.rows; y++){
            //Ako je filter value bio 0 tada postaje 50. Ako je filter value bio 255 i nije prošao provjeru tada postaje 100, inače 255
            if(this->lastMsg->second.at<u_char>(y,x) == 0){
                continue;
            }

            cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(y,x);

            //HUE
            if(hsv[0] >= this->hue[0] && hsv[0] <= this->hue[1]
            && hsv[1] >= this->sat[0] && hsv[1] <= this->sat[1]
            && hsv[2] >= this->val[0] && hsv[2] <= this->val[1]){
                this->lastMsg->second.at<u_char>(y,x) = 255;
            }
            else{
                this->lastMsg->second.at<u_char>(y,x) = 70;
            }
        }
    }

    //Tu update napraviti tako da se uzima freezed frame ako treba...
    Util::mat2Texture(this->lastMsg->first, this->lastMsg->second, this->texture); // if freezed -> update....

    
    //printf("texture id = %d\n", this->texture);
    

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    //ImGui::ImageButton()
    if (ImGui::ImageButton("", (ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor))){

        ImGui::OpenPopup("choose range");
        //Make static frame
        
        
        //Util::mat2Texture(this->hsvColorChooserFrame, this->hsvColorChooserTexture);
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

        
        ImGui::EndPopup();
    }



    this->mutex.unlock();
    return;
}



void NodeTest::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if(connector->connectorMessageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        //printf("Message came\n");
        this->resolution[0]=msg->data->first.cols;
        this->resolution[1]=msg->data->first.rows;

        this->mutex.lock();
        
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

        }

        //create new filtered msg here based on set hue, saturation and value, change just filter part...
        //just update second part of filter, based on set hue sat and val.

        cv::Mat hsvFrame;
        cvtColor(msg->data->first, hsvFrame, cv::COLOR_BGR2HSV);

        for(int x = 0; x < hsvFrame.cols; x++){
            for(int y = 0; y < hsvFrame.rows; y++){
                //Ako je filter value bio 0 tada postaje 50. Ako je filter value bio 255 i nije prošao provjeru tada postaje 100, inače 255
                if(msg->data->second.at<u_char>(y,x) == 0){
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

        this->sendAll(msg);
        this->mutex.unlock();


        
        
    }
    
    
    
}


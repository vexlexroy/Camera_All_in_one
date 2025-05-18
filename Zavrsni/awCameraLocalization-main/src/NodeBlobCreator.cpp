#include "NodeBlobCreator.hpp"
#include "NodeBase.hpp"
#include "Enums.hpp"
#include "ElementFactory.hpp"
#include "Message.hpp"
#include "ConnectorBase.hpp"
#include "Util.hpp"
#include "Structs.hpp"
#include "GlobalParams.hpp"


#include <memory>
#include <vector>
#include <iostream>
#include <imgui.h>
#include <opencv2/core/matx.hpp>
#include <algorithm>
#include <imnodes.h>

NodeBlobCreator::NodeBlobCreator(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEBLOBCREATOR;
}

std::shared_ptr<NodeBase> NodeBlobCreator::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeBlobCreator>(uniqueId);
}



std::vector<Enums::MessageType> NodeBlobCreator::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeBlobCreator::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::BLOB);
    
    return outMessageTypes;
}


std::string NodeBlobCreator::getDescription(){
    return "Ovo je blob creator node";
}

std::string NodeBlobCreator::getName(){
    return "Blob-creator";
}

Enums::NodeType NodeBlobCreator::getType(){
    return Enums::NodeType::NODEBLOBCREATOR;
}





void NodeBlobCreator::drawNodeParams(){
    //printf("Is sizeHovered = %d, center = %d, dispersity = %d, hu = %d\n", this->isSizeHovered, this->isCenterHovered, this->isDispersityHovered, this->isHuHovered);
    ImGui::Text("Contour-mode:");
    ImGui::SameLine();
    ImGui::PushItemWidth(130);
    ImGui::Combo("##contourMode", &this->contourModeCurrent, this->contourMode, IM_ARRAYSIZE(this->contourMode));
    ImGui::PopItemWidth();

    ImGui::Text("Contour-aprox:");
    ImGui::SameLine();
    ImGui::PushItemWidth(130);
    ImGui::Combo("##contourAprox", &this->contourAproxCurrent, this->contourAprox, IM_ARRAYSIZE(this->contourAprox));
    ImGui::PopItemWidth();



    
    ImGui::Text("Star");
    ImGui::SameLine();
    ImGui::ColorEdit4("##colorStar", (float*)&this->colorStar, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);


    //ImGui::Columns(2);
    //ImGui::SetColumnWidth(-1, 90);
    


    //CENTER
    ImGui::BeginGroup();
    ImGui::Text("Center");
    ImGui::SameLine();
    ImGui::BeginDisabled();
    bool alwaysOn = true;
    ImGui::Checkbox("##centerAlwaysOn", &alwaysOn);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::ColorEdit4("##colorCenter", (float*)&this->colorCenter, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    ImGui::SameLine();
    ImGui::PushItemWidth(130);
    ImGui::Combo("##center method", &this->objectCenterMethodCurrent, this->objectCenterMethod, IM_ARRAYSIZE(this->objectCenterMethod));
    ImGui::PopItemWidth();
    ImGui::EndGroup();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("center combo box is being hovered.");
        this->isCenterHovered = true;
    }
    else{
        this->isCenterHovered = false;
    }


    //SIZE
    ImGui::BeginGroup();
    ImGui::Text("Size");
    ImGui::SameLine();
    ImGui::Checkbox("##size", &this->isSizeEnabled);
    if(this->isSizeEnabled){
        ImGui::SameLine();
        ImGui::ColorEdit4("##colorSize", (float*)&this->colorSize, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine();
        ImGui::PushItemWidth(130);
        ImGui::Combo("##size method", &this->objectSizeMethodCurrent, this->objectSizeMethod, IM_ARRAYSIZE(this->objectSizeMethod));
        ImGui::PopItemWidth();
        
    }
    ImGui::EndGroup();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("size combo box is being hovered.");
        this->isSizeHovered = true;
    }
    else{
        this->isSizeHovered = false;
    }


    //DISPERSITY
    ImGui::BeginGroup();
    ImGui::Text("Dispersity");
    ImGui::SameLine();
    ImGui::Checkbox("##dispersity", &this->isDispersityEnabled);
    if(this->isDispersityEnabled){
        ImGui::SameLine();
        ImGui::ColorEdit4("##colorDispersity", (float*)&this->colorDispersity, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine();
        ImGui::PushItemWidth(130);
        ImGui::Combo("##dispersity method", &this->objectDispersityMethodCurrent, this->objectDispersityMethod, IM_ARRAYSIZE(this->objectDispersityMethod));
        ImGui::PopItemWidth();
    }
    ImGui::EndGroup();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("dispersity combo box is being hovered.");
        this->isDispersityHovered = true;
    }
    else{
        this->isDispersityHovered = false;
    }
    
    
    //HU
    ImGui::BeginGroup();
    ImGui::Text("Hu");
    ImGui::SameLine();
    ImGui::Checkbox("##huEnable", &this->isHuEnabled);
    if(this->isHuEnabled){
        ImGui::SameLine();
        ImGui::ColorEdit4("##colorHu", (float*)&this->colorHu, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    }
    ImGui::EndGroup();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Hue is being hovered.");
        this->isHuHovered = true;
    }
    else{
        this->isHuHovered = false;
    }

    ImGui::PushItemWidth(50);
    ImGui::DragInt("maxBlobCnt", &this->maxBlobCnt, 0.1, 1, 10);

    //ImGui::DragFloat("floatMin", &this->mi, 0.1, 1, 10);
    ImGui::PopItemWidth();


    if (strcmp(this->contourMode[this->contourModeCurrent], "RETR_EXTERNAL") == 0) {
        this->retrievalMode = cv::RetrievalModes::RETR_EXTERNAL;
    }
    else if(strcmp(this->contourMode[this->contourModeCurrent], "RETR_CCOMP") == 0){
        this->retrievalMode = cv::RetrievalModes::RETR_CCOMP;
    }


    //const char* contourAprox[4] = {"NONE", "SIMPLE", "TC89_L1", "TC89_KCOS"};
    if (strcmp(this->contourAprox[this->contourAproxCurrent], "NONE") == 0) {
        this->aproxMode = cv::ContourApproximationModes::CHAIN_APPROX_NONE;
    }
    else if (strcmp(this->contourAprox[this->contourAproxCurrent], "SIMPLE") == 0) {
        this->aproxMode = cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE;
    }
    else if (strcmp(this->contourAprox[this->contourAproxCurrent], "TC89_L1") == 0) {
        this->aproxMode = cv::ContourApproximationModes::CHAIN_APPROX_TC89_L1;
    }
    else if (strcmp(this->contourAprox[this->contourAproxCurrent], "TC89_KCOS") == 0) {
        this->aproxMode = cv::ContourApproximationModes::CHAIN_APPROX_TC89_KCOS;
    }



    
    //printf("Contours = %d\n", this->contours.size()); //:)


}

void NodeBlobCreator::drawNodeWork(){
    this->mutex.lock();
    
    if(this->rawPicture != nullptr){

        //Make rawPicture moreTransparent...

        cv::Mat result = (*this->rawPicture).clone(); // Make a copy to draw on
        ImGui::Text("center = %d\n", this->objectCenterMethodCurrent);

        
        for(int i = 0; i < this->blobs.size(); i++){
            

            if(this->isCenterHovered){
                //Za circle prikaži krug
                

                if(this->blobs[i].middle.x < this->resolution[0] && this->blobs[i].middle.y < this->resolution[1] && this->blobs[i].middle.x > 0 && this->blobs[i].middle.y > 0){
                    Util::drawCenter(result, cv::Point(this->blobs[i].middle.x, this->blobs[i].middle.y), 15, this->colorCenter, 2);
                }
            }
            else if(this->isSizeHovered && this->isSizeEnabled){

            }

            else if(this->isDispersityHovered && this->isDispersityEnabled){
                
            }
            else{ // show combined results
                //Center -> show small x
                
                //NE RADI TU SAM STAO!!!!!!!!!!

                //printf("Blobs size = %d\n", this->blobs.size());
                //cv::line(result, cv::Point(this->blobs[i].middle.x, this->blobs[i].middle.y), cv::Point(this->blobs[i].middle.x + 20, this->blobs[i].middle.y + 20), )
                
                
                if(this->blobs[i].middle.x < this->resolution[0] && this->blobs[i].middle.y < this->resolution[1] && this->blobs[i].middle.x > 0 && this->blobs[i].middle.y > 0){
                    Util::drawCenter(result, cv::Point(this->blobs[i].middle.x, this->blobs[i].middle.y), 20, this->colorStar, 2);
                }

                //Size/dispersity -> show star -> base radius 
                //dispersity -> define measure of spikes min/max difference... (fixed pixels...)


                // Center is defined with x.
                
                //Area is defined with circle

                //Dispersity is defined by min/max star... (visualy satisfiing)
            }
        }
        
        cv::Scalar color(255, 0, 0);
        if(this->circles.size() > 0 && strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "circleMiddle") == 0){
            
            
            for(int i = 0; i < this->circles.size(); i++){
                cv::circle(result, this->circles[i].first, this->circles[i].second, color, 2);
            }
            

            //cv::drawContours(result, this->contours, -1, cv::Scalar(0, 255, 0), 2); // Draw all contours in green with thickness 2
            
        }
        else if((strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "rectangleMiddle") == 0) && this->rectangles.size() > 0){
            
            for(int i = 0; i < this->rectangles.size(); i++){
                //cv::RotatedRect(result, this->rectangles[i], color, 2);

                cv::Point2f vertices[4];
                
                this->rectangles[i].second.points(vertices);

                for (int i = 0; i < 4; ++i) {
                    cv::line(result, vertices[i], vertices[(i + 1) % 4], color, 2);
                }
            }
        }
        //printf("Rectangle method, size = %d\n", this->rectangles.size());
        
        
           


        Util::mat2Texture(result, this->texture);
        ImGui::Image((ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor));
    }
    
    
    this->mutex.unlock();
    return;
}


void NodeBlobCreator::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if(connector->connectorMessageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> rawMsg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        
        this->mutex.lock();
        this->resolution[0]=rawMsg->data->first.cols;
        this->resolution[1]=rawMsg->data->first.rows;
        //I can consume raw Msg because it is last in chain...


        //std::unique_ptr<std::pair<cv::Mat, cv::Mat>> msgCpy = std::make_unique<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));
        //this->filteredMsg = std::make_unique<cv::Mat>(msgCpy->first.clone());


        //const char* contourMode[2] = {"RETR_EXTERNAL", "RETR_CCOMP"};
        
        
        //printf("Mode = %d, aprox = %d\n", this->retrievalMode, this->aproxMode);
        //CREATE CONTOUR...
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> contoursHierarchy;

        //printf("Retr mode = %d, aprox mode = %d\n", this->retrievalMode, this->aproxMode);
        
        cv::findContours(rawMsg->data->second, contours, contoursHierarchy, this->retrievalMode, this->aproxMode);
        
        //Create finalStructure based on contours number
        std::vector<Structs::BlobDescription> blob; // data structure that will be sent ..., i need to fill that accordingly...


        
        std::vector<std::pair<int, std::vector<int>>> myHierarchy;
        
        for(int i = 0; i < contours.size(); i++){
            if(contoursHierarchy[i][3] == -1){ // has no parent, is outbounding
                
                //Create entry
                std::pair<int, std::vector<int>> newContourPair;
                int outContourId = i;
                newContourPair.first = outContourId;

                int childId = contoursHierarchy[i][2]; // first child
                while(childId >= 0){
                    newContourPair.second.push_back(childId);
                    childId = contoursHierarchy[childId][0]; // newt sibling
                }

                myHierarchy.push_back(newContourPair);
            }
        }
        
        

        //const char* objectCenterMethod[4] = {"arithmeticMiddle", "weightedArithmeticMiddle", "circleMiddle", "rectangleMiddle"};
        //const char* objectSizeMethod[3] = {"pixelArea", "circleArea", "rectangleArea"};
        //const char* objectDispersityMethod[3] = {"circleAreaPart", "rectangleAreaPart", "densityQuotient"};

        //+ circleMiddle, circleArea
        bool shouldFindCircle = (strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "circleMiddle") == 0)
                            ||  this->isSizeEnabled && (strcmp(this->objectSizeMethod[this->objectSizeMethodCurrent], "circleArea") == 0)
                            ||  this->isDispersityEnabled && (strcmp(this->objectDispersityMethod[this->objectDispersityMethodCurrent], "circleAreaPart") == 0);
        
        
        
        //+ rectangleMiddle, rectangleArea
        bool shouldFindRectangle = (strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "rectangleMiddle") == 0)
                            ||  this->isSizeEnabled && (strcmp(this->objectSizeMethod[this->objectSizeMethodCurrent], "rectangleArea") == 0)
                            ||  this->isDispersityEnabled && (strcmp(this->objectDispersityMethod[this->objectDispersityMethodCurrent], "rectangleAreaPart") == 0);
        
        
        //+ arithmeticMiddle, pixelArea, circleAreaPart, rectangleAreaPart
        bool shouldCalcMoments = (strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "arithmeticMiddle") == 0)
                            ||     this->isSizeEnabled && (strcmp(this->objectSizeMethod[this->objectSizeMethodCurrent], "pixelArea") == 0)
                            ||     this->isDispersityEnabled && (strcmp(this->objectDispersityMethod[this->objectDispersityMethodCurrent], "rectangleAreaPart") == 0)
                            ||      this->isDispersityEnabled && (strcmp(this->objectDispersityMethod[this->objectDispersityMethodCurrent], "circleAreaPart") == 0)
                            ||  this->isHuEnabled;
        
        //-huMoments, -weightedMiddle
        //bool shouldCalcHuMoments = this->isHuEnabled;

        //-weightedMiddle, dispersityAreaPart. On the end...
        //printf("Is sizeHovered = %d, dispersity = %d, hu = %d\n", this->isSizeEnabled, this->isDispersityEnabled, this->isHuEnabled);
        //printf("shouldFindCircle = %d, shouldFindRectangle= %d, shouldFindObjectPixelArea = %d, shouldCalcMoments = %d, shouldCalcHuMoments = %d\n", 
        //shouldFindCircle, shouldFindRectangle, shouldFindObjectPixelArea, shouldCalcMoments, shouldCalcHuMoments);
        
        //Always find rectangles...
        std::vector<std::pair<int, cv::RotatedRect>> rectangles;
        for(int i = 0; i < myHierarchy.size(); i++){


            int parentId = myHierarchy[i].first;
            std::vector<cv::Point> parentContour = contours[parentId];
            cv::RotatedRect rect = cv::minAreaRect(parentContour);
            

            rectangles.push_back(std::make_pair(myHierarchy[i].first, rect));
        }


        std::sort(rectangles.begin(), rectangles.end(),[](const std::pair<int, cv::RotatedRect>& a, const std::pair<int, cv::RotatedRect>& b) {
            return a.second.size.width * a.second.size.height > b.second.size.width * b.second.size.height; // Sort in descending order
        });

        if(rectangles.size() > this->maxBlobCnt){
            rectangles.resize(this->maxBlobCnt);
        }
        
        myHierarchy.erase(std::remove_if(myHierarchy.begin(), myHierarchy.end(), [&rectangles](const std::pair<int, std::vector<int>>& myHierarchyElem) {
            // Check if myHierarchyElem.first exists in rectangles
            for (const auto& rect : rectangles) {
                if (rect.first == myHierarchyElem.first) {
                    return false; // Keep this element
                }
            }
            return true; // Remove this element
        }), myHierarchy.end());

        



        std::vector<std::pair<cv::Point2f, float>> circles;
        if(shouldFindCircle){ // only for outer contours find all enclosing circles...
            for(int i = 0; i < myHierarchy.size(); i++){
                cv::Point2f newCenter;
                float newRadius;

                int parentId = myHierarchy[i].first;
                std::vector<cv::Point> parentContour = contours[parentId];
                cv::minEnclosingCircle(parentContour, newCenter, newRadius);
                circles.push_back(std::make_pair(newCenter, newRadius));
            }    
        }




        std::vector<std::pair<cv::Moments, std::vector<cv::Moments>>> moments;
        if(shouldCalcMoments){
            for(int i = 0; i < myHierarchy.size(); i++){
                int parentId = myHierarchy[i].first;

                std::pair<cv::Moments, std::vector<cv::Moments>> momentsPair;

                cv::Moments parentMoment = cv::moments(contours[i]);
                momentsPair.first = parentMoment;

                std::vector<cv::Moments> childMoments;
                for(int j = 0; i < myHierarchy[i].second.size(); j++){
                    int childId = myHierarchy[i].second[j];
                    
                    cv::Moments childMoment = cv::moments(contours[j]);
                    childMoments.push_back(childMoment);
                }
                momentsPair.second = childMoments;

                moments.push_back(momentsPair);
            }
            //printf("Moments calculated, size = %d\n" , moments.size());
        }
        

        //Calculate all params from data i just collected...
        std::vector<Structs::BlobDescription> blobs;
        //Calculate object middle...
        //const char* objectCenterMethod[4] = {"arithmeticMiddle", "weightedArithmeticMiddle", "circleMiddle", "rectangleMiddle"};
        
        for(int i = 0; i < myHierarchy.size(); i++){
            int parentId = myHierarchy[i].first;
            std::vector<int> childId = myHierarchy[i].second;

            Structs::BlobDescription newBlobDescription;
            //Filling random data...

            newBlobDescription.size = -1;
            newBlobDescription.middle = ImVec2(-1.0, -1.0);
            newBlobDescription.dispersity = -1.0;
            newBlobDescription.huMoments[0] = -1.0;
            newBlobDescription.huMoments[1] = -1.0;
            newBlobDescription.huMoments[2] = -1.0;
            newBlobDescription.huMoments[3] = -1.0;
            newBlobDescription.huMoments[4] = -1.0;
            newBlobDescription.huMoments[5] = -1.0;
            newBlobDescription.huMoments[6] = -1.0;
            

            //SETTING MIDDLE
            if(strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "arithmeticMiddle") == 0){
                
                //printf("Trying arithmetic middle1, %d\n", this->objectCenterMethodCurrent);
                double xSum = 0;
                double ySum = 0;
                double pixelsNum = 0;

                pixelsNum += moments[i].first.m00;
                xSum += moments[i].first.m10;
                ySum += moments[i].first.m01;
                for(int childId; childId < moments[i].second.size(); childId++){
                    pixelsNum -= moments[i].second[childId].m00;  
                    xSum -= moments[i].second[childId].m10;
                    ySum -= moments[i].second[childId].m01;
                }
                if(pixelsNum > 0){
                    newBlobDescription.middle.x = xSum/pixelsNum;
                    newBlobDescription.middle.y = ySum/pixelsNum;
                }
                else{
                    newBlobDescription.middle.x = 50;
                    newBlobDescription.middle.y = 50;
                }
                
            }
            
            else if(strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "circleMiddle") == 0){
                newBlobDescription.middle.x = circles[i].first.x;
                newBlobDescription.middle.y = circles[i].first.y;
            }
            else if(strcmp(this->objectCenterMethod[this->objectCenterMethodCurrent], "rectangleMiddle") == 0){
                newBlobDescription.middle.x = rectangles[i].second.center.x;
                newBlobDescription.middle.y = rectangles[i].second.center.y;
            }

            
            //SETTING SIZE
            //const char* objectSizeMethod[3] = {"pixelArea", "circleArea", "rectangleArea"};
            if(this->isSizeEnabled){
                if(strcmp(this->objectSizeMethod[this->objectSizeMethodCurrent], "pixelArea") == 0){
                    
                    double pixelsNum = 0;
                    pixelsNum += moments[i].first.m00;
                    for(int childId = 0; childId < moments[i].second.size(); childId++){
                        pixelsNum -= moments[i].second[childId].m00;  
                    }
                    

                    newBlobDescription.size = pixelsNum;
                    
                }
                else if(strcmp(this->objectSizeMethod[this->objectSizeMethodCurrent], "circleArea") == 0){
                    newBlobDescription.size = circles[i].second;
                }
                else if(strcmp(this->objectSizeMethod[this->objectSizeMethodCurrent], "rectangleArea") == 0){
                    newBlobDescription.size = rectangles[i].second.boundingRect2f().area();
                }
            }
            
            
            //SETTING DISPERSITY
            //const char* objectDispersityMethod[3] = {"circleAreaPart", "rectangleAreaPart", "densityQuotient"};
            if(this->isDispersityEnabled){
                if(strcmp(this->objectDispersityMethod[this->objectDispersityMethodCurrent], "circleAreaPart") == 0){
                    //printf("Moments num = %d, myHierarchySize = %d, shouldCalcMoments = %d, circlesSize = %d, shouldFindCircle = %d\n", moments.size(), myHierarchy.size(), shouldCalcMoments, circles.size(), shouldFindCircle);

                    double pixelsNum = 0;
                    pixelsNum += moments[i].first.m00;
                    for(int childId = 0; childId < moments[i].second.size(); childId++){
                        pixelsNum -= moments[i].second[childId].m00;  
                    }
                    
                    double circleRadius = circles[i].second;
                    double circleArea = circleRadius*circleRadius*3.14159265358979323846;

                    if(circleArea > 0){
                        newBlobDescription.dispersity = 1 - pixelsNum/circleArea;
                    }
                    else{
                        newBlobDescription.dispersity = 1;
                    }
                    //printf("Did not execute\n");
                }
                else if(strcmp(this->objectDispersityMethod[this->objectDispersityMethodCurrent], "rectangleAreaPart") == 0){
                    double pixelsNum = 0;
                    pixelsNum += moments[i].first.m00;
                    for(int childId = 0; childId < moments[i].second.size(); childId++){
                        pixelsNum -= moments[i].second[childId].m00;  
                    }

                    double rectangleArea = rectangles[i].second.boundingRect2f().area();

                    
                    if(rectangleArea > 0){
                        newBlobDescription.dispersity = 1 - pixelsNum/rectangleArea;
                    }
                    else{
                        newBlobDescription.dispersity = 1;
                    }
                }
            }
            
            
            
            //SETTING HU (for now not set...)
            
            blobs.push_back(newBlobDescription);
        }
        

        

        //Update local params if needed...
        if(this->showNodeWork){
            
            //update rawPicture, rawFilter, workingContours, circleDef, rectAngleDef, weighted-filter
            //update middle, size, dispersity, hu
            this->rawPicture = std::make_unique<cv::Mat>(rawMsg->data->first);
            
            this->orderedContours = orderedContours;
            this->contours = contours;
            this->contoursHierarchy = contoursHierarchy;
            this->myHierarchy = myHierarchy;
            
            this->circles = circles;
            this->rectangles = rectangles;
            this->moments = moments;
            this->blobs = blobs;
        }
        

        //Create msg and send it...
        if(blobs.size() != 0){
            std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>> msg = std::make_shared<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(Enums::MessageType::BLOB, message);
            msg->data = std::make_shared<std::vector<Structs::BlobDescription>>(blobs);
            
            this->sendAll(msg);
        }
        

        this->mutex.unlock();
    } 
}


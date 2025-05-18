#include "NodeManualExtrinsic.hpp"
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
#include <boost/format.hpp>


NodeManualExtrinsic::NodeManualExtrinsic(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEMANUALEXTRINSIC;
}

std::shared_ptr<NodeBase> NodeManualExtrinsic::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeManualExtrinsic>(uniqueId);
}



std::vector<Enums::MessageType> NodeManualExtrinsic::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::PICTURE);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeManualExtrinsic::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    //outMessageTypes.push_back(Enums::MessageType::);
    
    return outMessageTypes;
}


std::string NodeManualExtrinsic::getDescription(){
    return "Ovaj node služi kako bi se manualno extrinzično kalibrrirale kamere";
}

std::string NodeManualExtrinsic::getName(){
    return "Node-manual-extrinsic";
}

Enums::NodeType NodeManualExtrinsic::getType(){
    return Enums::NodeType::NODEMANUALEXTRINSIC;
}

void NodeManualExtrinsic::drawNodeParams(){
    int blobGroupsCnt = 2;
    ImGui::PushItemWidth(50);


    //if world frame is nullptr but exist than apply right world frame
    std::shared_ptr<FrameBase> worldFrameTmp = GlobalParams::getInstance().getFrame(GlobalParams::getInstance().getWorldFrame());

    if(worldFrameTmp != nullptr)
    {
        this->worldFrame = worldFrameTmp;
    }

    bool anySelectableHovered = false;
    std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
    std::vector<std::shared_ptr<FrameCam>> openedCamFrames;
    int longestCamName = 15;
    for(int i = 0; i < camFrames.size(); i++){
        if(camFrames[i]->isConnected){
            openedCamFrames.push_back(camFrames[i]);
            if(camFrames[i]->frameNickName.size() > longestCamName){
                longestCamName = camFrames[i]->frameNickName.size();
            }
        }   
    }

    int numColumn = openedCamFrames.size()+1;
    int numRow = openedCamFrames.size();
    //printf("numColumn = %d, numRow = %d\n", numColumn, numRow);
    
    ImVec2 outer_size = ImVec2(longestCamName*7*numColumn, 16*numRow);
    if (numRow >= 1 && ImGui::BeginTable("camRelation", numColumn, ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable, outer_size)){

        ImGui::TableNextRow();
        for (int i = 0; i < numColumn; i++){
            ImGui::TableSetColumnIndex(i);
            //ImGui::PushID(i);
            //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            //ImGui::Checkbox("##checkall", &column_selected[column]);
            if(i == 0){
                ImGui::Selectable("ALL", &this->selectAll);
                if(ImGui::IsItemHovered()){
                    anySelectableHovered = true;
                    this->hoveredPair.first = "*";
                    this->hoveredPair.second = "*";
                }
                continue;
            }

            //static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
            if(camColor.count(openedCamFrames[i-1]->frameNickName) == 0){
                camColor[openedCamFrames[i-1]->frameNickName] = Util::getImVec4FromString(openedCamFrames[i-1]->frameNickName);
            }
            ImGui::Text(openedCamFrames[i-1]->frameNickName.c_str());
            ImGui::SameLine();

            ImGui::ColorEdit4(("##" + openedCamFrames[i-1]->frameNickName).c_str(), (float*)&camColor[openedCamFrames[i-1]->frameNickName], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            

        }

        
        for (int row = 0; row < numRow; row++){
            ImGui::TableNextRow();
            for (int column = 0; column < numColumn; column++){
                ImGui::TableSetColumnIndex(column);


                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));// Hover color
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f)); // Click pushed color

                

                if(column == 0){
                    ImGui::Selectable(openedCamFrames[row]->frameNickName.c_str());
                    bool is_pressed = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
                    if (is_pressed) {
                        this->worldFrame = openedCamFrames[row];
                        printf("World frame = %s\n", this->worldFrame->frameNickName);
                        //selected_item = item_id; // Set this item as selected on click
                    }

                    ImGui::PopStyleColor(2);
                    continue;
                }
                
                bool isPairSelected = false;
                for(int i = 0; i < this->selectedPair.size(); i++){
                    if((this->selectedPair[i].first == openedCamFrames[row]->frameNickName && this->selectedPair[i].second == openedCamFrames[column-1]->frameNickName)
                    || (this->selectedPair[i].first == openedCamFrames[column-1]->frameNickName && this->selectedPair[i].second == openedCamFrames[row]->frameNickName)){
                        isPairSelected = true;
                        break;
                    }
                }
                

                if(isPairSelected){
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,  ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.8f, 1.0f)));
                }
                //Calibration if size() == num -> ..., if not show percentage, else for now show -.



                std::string finalValue = "-";

    
                

                bool currPressed = ImGui::Selectable((finalValue + "##" + openedCamFrames[row]->frameNickName + openedCamFrames[column - 1]->frameNickName).c_str());
                if(ImGui::IsItemHovered()){
                    anySelectableHovered = true;
                    if(row > column - 1){
                        this->hoveredPair.first = openedCamFrames[row]->frameNickName;
                        this->hoveredPair.second = openedCamFrames[column-1]->frameNickName;
                    }
                    else{
                        this->hoveredPair.first = openedCamFrames[column-1]->frameNickName;
                        this->hoveredPair.second = openedCamFrames[row]->frameNickName;
                    }
                    
                }
                
                
                if(isPairSelected){
                    ImGui::PopStyleColor(2);
                }
                else{
                    ImGui::PopStyleColor(2);
                }

                if(currPressed){ // change that pair state...
                    //printf("Selected = %d\n", this->selectedPair.size());
                    printf("Curr pressed -> %s, %s\n", openedCamFrames[row]->frameNickName.c_str(), openedCamFrames[column-1]->frameNickName.c_str());
                    if(openedCamFrames[row]->frameNickName != openedCamFrames[column-1]->frameNickName)
                    {
                        this->triggerExternalCalibration = true;
                        this->externalCalibrationPair.first = openedCamFrames[row]->frameNickName;
                        this->externalCalibrationPair.second = openedCamFrames[column-1]->frameNickName;
                    }

                    if (isPairSelected) { // If the pair is already selected
                        // Remove both entries from std::vector
                        this->selectedPair.erase(
                            std::remove_if(
                                this->selectedPair.begin(),
                                this->selectedPair.end(),
                                [frame1 = openedCamFrames[row]->frameNickName, frame2 = openedCamFrames[column - 1]->frameNickName](const std::pair<std::string, std::string>& p) {
                                    return (p.first == frame1 && p.second == frame2) || (p.first == frame2 && p.second == frame1);
                                }
                            ),
                            this->selectedPair.end()
                        );
                    }
                    else{
                        if(row > column - 1){
                            this->selectedPair.push_back(std::make_pair(openedCamFrames[row]->frameNickName, openedCamFrames[column-1]->frameNickName));
                        }
                        else{
                            this->selectedPair.push_back(std::make_pair(openedCamFrames[column-1]->frameNickName, openedCamFrames[row]->frameNickName));
                        }
                        
                    }
                    
                }
                /*
                //to cam size, and to custom frame size, and then last row is + button...


                
                // Draw the selectable item
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));// Hover color
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f)); // Click pushed color
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,  ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.8f, 0.2f))); // default color

                ImGui::Selectable("Bla");
                bool is_pressed = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
                if (is_pressed) {
                    printf("Clicked\n");
                    //selected_item = item_id; // Set this item as selected on click
                }
                ImGui::PopStyleColor(4);
                

                char buf[32];
                sprintf(buf, "Cell %d,%d", column, row);
                ImGui::TableSetColumnIndex(column);
                ImGui::Text("Bla");
                */
                //ImGui::Selectable(buf, column_selected[column]);
            }
        }

        if(!anySelectableHovered){
            this->hoveredPair.first = "";
            this->hoveredPair.second = "";
        }
        
        //printf("Hovered = %s, %s\n", this->hoveredPair.first.c_str(), this->hoveredPair.second.c_str());
        
        ImGui::EndTable();
    }
    //ImGui::DragInt("group-cnt", &this->blobGroupsCnt, 1.0, 10, 2000);
    //ImGui::Separator();
    //ImGui::DragInt("viewerTime", &this->viewerTime, 1.0, 100, 1000);
    ImGui::PopItemWidth();
}

void NodeManualExtrinsic::drawNodeWork(){
    this->mutex.lock();
    
    //if any pair selected -> show 
    std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
    std::vector<std::shared_ptr<FrameCustom>> camCustom = GlobalParams::getInstance().getCamCustomFrames();


    if(this->triggerExternalCalibration)
    {
        printf("Trigggerd\n");
        ImGui::OpenPopup("externalCalibrationPopup");
        this->triggerExternalCalibration = false;
        this->pairList.clear();
        this->pairNew.first.x = 0;
        this->pairNew.first.y = 0;
        this->pairNew.second.x = 0;
        this->pairNew.second.y = 0;
        

        std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
        std::vector<std::shared_ptr<FrameCam>> openedCamFrames;
        for(int i = 0; i < camFrames.size(); i++){
            if(camFrames[i]->isConnected){
                openedCamFrames.push_back(camFrames[i]);
            }   
        }

        std::shared_ptr<FrameCam> cam1;
        std::shared_ptr<FrameCam> cam2;

        // Find the cameras corresponding to the matched results
        for (int j = 0; j < openedCamFrames.size(); j++) {
            if (openedCamFrames[j]->frameNickName == this->externalCalibrationPair.first) {
                cam1 = openedCamFrames[j];
            } else if (openedCamFrames[j]->frameNickName == this->externalCalibrationPair.second) {
                cam2 = openedCamFrames[j];
            }
        }

        if (!cam1 || !cam2) {
            std::cerr << "Error: Unable to find corresponding cameras for the matched results." << std::endl;
            return;
        }


        this->hue1[0] = cam1->colorThresholdParams.hue[0];
        this->hue1[1] = cam1->colorThresholdParams.hue[1];
        this->sat1[0] = cam1->colorThresholdParams.separation[0];
        this->sat1[1] = cam1->colorThresholdParams.separation[1];
        this->val1[0] = cam1->colorThresholdParams.value[0];
        this->val1[1] = cam1->colorThresholdParams.value[1];


        this->hue2[0] = cam2->colorThresholdParams.hue[0];
        this->hue2[1] = cam2->colorThresholdParams.hue[1];
        this->sat2[0] = cam2->colorThresholdParams.separation[0];
        this->sat2[1] = cam2->colorThresholdParams.separation[1];
        this->val2[0] = cam2->colorThresholdParams.value[0];
        this->val2[1] = cam2->colorThresholdParams.value[1];
        

        //Try to load extrinsic params of both cameras...
        std::vector<std::shared_ptr<FrameRelation>> relations = GlobalParams::getInstance().getCamRelations();
        this->currFrameRelation = nullptr;
        for(int i = 0; i < relations.size(); i++)
        {
            if(((relations[i]->frame_src->frameNickName == cam1->frameNickName)
            && (relations[i]->frame_destination->frameNickName == cam2->frameNickName))
            || ((relations[i]->frame_src->frameNickName == cam2->frameNickName)
            && (relations[i]->frame_destination->frameNickName == cam1->frameNickName)))
            {
                this->currFrameRelation = relations[i];
            }
        }



        
    }

    if (ImGui::BeginPopup("externalCalibrationPopup")){

        

        std::vector<std::shared_ptr<FrameCam>> openedCamFrames;
        for(int i = 0; i < camFrames.size(); i++){
            if(camFrames[i]->isConnected){
                openedCamFrames.push_back(camFrames[i]);
            }   
        }

        std::shared_ptr<FrameCam> cam1;
        std::shared_ptr<FrameCam> cam2;

        // Find the cameras corresponding to the matched results
        for (int j = 0; j < openedCamFrames.size(); j++) {
            if (openedCamFrames[j]->frameNickName == this->externalCalibrationPair.first) {
                cam1 = openedCamFrames[j];
            } else if (openedCamFrames[j]->frameNickName == this->externalCalibrationPair.second) {
                cam2 = openedCamFrames[j];
            }
        }

        if (!cam1 || !cam2) {
            std::cerr << "Error: Unable to find corresponding cameras for the matched results." << std::endl;
            return;
        }



        cv::Mat cameraMatrix1 = cam1->intrinsicParams.intrinsicMatrix;
        cv::Mat distCoeffs1 = cam1->intrinsicParams.distortionCoef;
        cv::Mat cameraMatrix2 = cam2->intrinsicParams.intrinsicMatrix;
        cv::Mat distCoeffs2 = cam2->intrinsicParams.distortionCoef;

        
        if (ImGui::Checkbox("##Enable Feature", &this->showtargetFrameCordSystem)) {
            // This block executes when the checkbox is clicked
            if (this->showtargetFrameCordSystem) {
                // Checkbox was turned ON
                printf("Feature enabled\n");
            } else {
                // Checkbox was turned OFF
                printf("Feature disabled\n");
            }
        }
        ImGui::SameLine();
        ImGui::Text("World-frame");
        ImGui::SameLine();
        std::vector<std::string> worldFrameCandidates;
        int cnt = 0;
        int worldFrameIndex = 0;

        for(int i = 0; i < camFrames.size(); i++)
        {
            if(GlobalParams::getInstance().getWorldFrame() == camFrames[i]->frameNickName)
            {
                worldFrameIndex = cnt;
            }
            cnt++;
            worldFrameCandidates.push_back(camFrames[i]->frameNickName);
        }
        for(int i = 0; i < camCustom.size(); i++)
        {
            if(GlobalParams::getInstance().getWorldFrame() == camCustom[i]->frameNickName)
            {
                worldFrameIndex = cnt;
            }
            cnt++;
            worldFrameCandidates.push_back(camCustom[i]->frameNickName);
        }


        if (ImGui::BeginCombo("##World-frame", worldFrameCandidates[worldFrameIndex].c_str())) {
            // Loop through all items in the vector
            for (int i = 0; i < worldFrameCandidates.size(); i++) {
                // Display each item as selectable
                const bool isSelected = (worldFrameIndex == i); // Check if the item is selected
                if (ImGui::Selectable(worldFrameCandidates[i].c_str(), isSelected)) {
                    worldFrameIndex = i; // Update the selected item index

                    GlobalParams::getInstance().setWorldFrame(worldFrameCandidates[i]);
                    this->worldFrame = GlobalParams::getInstance().getFrame(worldFrameCandidates[i]);
                }

                // Set the initial focus when opening the combo
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }


        ImGui::SameLine();
        if(this->customFrameProcedureCnt == -1)
        {
            if(ImGui::Button("+", ImVec2(15, 15)))
            {
                if(this->currFrameRelation == nullptr)
                {
                    ImGui::OpenPopup("warning-first-calibrate-extrinsicaly");
                }
                else
                {
                    this->calibrate_extrinsic_flag = false;
                    this->customFrameProcedureCnt = 0;
                }
            }
        }
        else
        {
            if(ImGui::Button("-", ImVec2(15, 15)))
            {
                this->customFrameProcedureCnt = -1;
            }
        }


        if(this->customFrameProcedureCnt == 6) //means it is done -> now add button finish
        {
            ImGui::SameLine();
            if(ImGui::Button("Create", ImVec2(50, 15)))
            {
                ImGui::OpenPopup("Custom frame name");
            }
        }

        if(ImGui::BeginPopup("warning-first-calibrate-extrinsicaly"))
        {
            ImGui::Text("First extrinsicaly calibrate cameras in order to make custom frame out of them");
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Custom frame name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            //check if input text is valid.
            bool doCustomFrameWithSameNameExist = false;
            std::shared_ptr<FrameCustom> existantFrame = nullptr;
            bool isValid = true;

            for(int i = 0; i < camFrames.size(); i++)
            {
                if(camFrames[i]->frameNickName == std::string(this->customFrameInputText))
                {
                    isValid = false;
                    break;
                }
            }
            for(int i = 0; i < camCustom.size(); i++)
            {
                if(camCustom[i]->frameNickName == std::string(this->customFrameInputText))
                {
                    existantFrame = camCustom[i];
                    doCustomFrameWithSameNameExist = true;
                    break;
                }
            }
            if(std::string(this->customFrameInputText).empty())
            {
                isValid = false;
            }


            if(isValid)
            {
                if(doCustomFrameWithSameNameExist) // make it yellow
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 0.0f, 0.2f)); // Yellow background
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9f, 0.9f, 0.2f, 0.2f)); // Slightly darker yellow when hovered
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.8f, 0.8f, 0.1f, 0.2f)); // Different yellow when active
                }
                else // make it green
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 1.0f, 0.0f, 0.2f)); // Green background
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.2f, 0.8f, 0.2f, 0.2f)); // Slightly darker green when hovered
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.1f, 0.9f, 0.1f, 0.2f)); // Different green when active
                }
                
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 0.2f)); // Green background
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.2f)); // Slightly darker green when hovered
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.1f, 0.1f, 0.2f)); // Different green when active
            }
            //if not valid -> text area is red, otherwise it is green
            
            ImGui::InputText("##TextBox", this->customFrameInputText, IM_ARRAYSIZE(this->customFrameInputText));
            ImGui::PopStyleColor(3);
            

            

            if(isValid && doCustomFrameWithSameNameExist && ImGui::Button("Replace"))
            {
                printf("Should be replaced\n");
                std::shared_ptr<FrameCustom> oldCustomFrame = GlobalParams::getInstance().getCustomFrame(this->customFrameInputText);

                if(oldCustomFrame != nullptr)
                {
                    oldCustomFrame->cam1Nickname = cam1->frameNickName;
                    oldCustomFrame->cam2Nickname = cam2->frameNickName;
                    
                    //Custom frame is shanged

                    //Now i must get relation that contains this custom frame and modify it as well
                    std::vector<std::shared_ptr<FrameRelation>> camRelations = GlobalParams::getInstance().getCamRelations();

                    for(int i = 0; i < camRelations.size(); i++)
                    {
                        if(camRelations[i]->frame_destination->frameNickName == oldCustomFrame->frameNickName) // this is the one
                        {  
                            camRelations[i]->frame_src = cam1;
                            camRelations[i]->frame_destination = oldCustomFrame;
                            camRelations[i]->distance_between_cams_in_cm = 1;
                            camRelations[i]->transformation_matrix_reprojection_error = 1;


                            for(int row = 0; row < 3; ++row) {
                                for (int col = 0; col < 3; ++col) {
                                    camRelations[i]->transformation_matrix.at<double>(row, col) = this->custom_frame_rotationMatrix.at<float>(row, col);
                                }
                            }

                            // Copy translation vector (3x1) to the last column of the first three rows
                            camRelations[i]->transformation_matrix.at<double>(0, 3) = this->custom_frame_translationVector.x;
                            camRelations[i]->transformation_matrix.at<double>(1, 3) = this->custom_frame_translationVector.y;
                            camRelations[i]->transformation_matrix.at<double>(2, 3) = this->custom_frame_translationVector.z;

                            this->customFrameProcedureCnt = -1;
                            break;
                        }
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            if(isValid && !doCustomFrameWithSameNameExist && ImGui::Button("Add"))
            {
                //Create new custom frame!
                GlobalParams::getInstance().createCustomFrame(this->customFrameInputText, cam1->frameNickName, cam2->frameNickName);
                //create new relation...
                std::shared_ptr<FrameCustom> newCustomFrame = GlobalParams::getInstance().getCustomFrame(this->customFrameInputText);
                
                if(newCustomFrame != nullptr)
                {
                    std::shared_ptr<FrameRelation> newFrameRelation = std::make_shared<FrameRelation>();
                    newFrameRelation->frame_src = cam1;
                    newFrameRelation->frame_destination = newCustomFrame;

                    

                    //rotation matrix must be in good format..., and translation must be normalized, and new parameter distance_between_cams_in_cm should exist 

                    newFrameRelation->transformation_matrix = cv::Mat::eye(4, 4, CV_64F); // Initialize as identity matrix
                    cv::Mat columnCorrectRotationMatrix = this->custom_frame_rotationMatrix.clone().t();

                    printf("Prosao1\n");
                    for (int row = 0; row < 3; ++row) {
                        for (int col = 0; col < 3; ++col) {
                            newFrameRelation->transformation_matrix.at<double>(row, col) = columnCorrectRotationMatrix.at<float>(row, col);
                        }
                    }
                    
                    // Copy translation vector (3x1) to the last column of the first three rows
                    cv::Mat currentTranslationVector = cv::Mat::zeros(3, 1, CV_32F);

                    // Fill it with values from custom_frame_translationVector
                    currentTranslationVector.at<float>(0, 0) = this->custom_frame_translationVector.x; // X
                    currentTranslationVector.at<float>(1, 0) = this->custom_frame_translationVector.y; // Y
                    currentTranslationVector.at<float>(2, 0) = this->custom_frame_translationVector.z; // Z

                    printf("Prosao1\n");


                    cv::Mat correctTranslationVector = columnCorrectRotationMatrix * (-currentTranslationVector);
                    double length = std::sqrt(correctTranslationVector.at<float>(0, 0) * correctTranslationVector.at<float>(0, 0) +
                              correctTranslationVector.at<float>(1, 0) * correctTranslationVector.at<float>(1, 0) +
                              correctTranslationVector.at<float>(2, 0) * correctTranslationVector.at<float>(2, 0));


                    correctTranslationVector.at<float>(0, 0) /= length;  // Normalize X
                    correctTranslationVector.at<float>(1, 0) /= length;  // Normalize Y
                    correctTranslationVector.at<float>(2, 0) /= length;  // Normalize Z

                    newFrameRelation->transformation_matrix.at<double>(0, 3) = correctTranslationVector.at<float>(0, 0);
                    newFrameRelation->transformation_matrix.at<double>(1, 3) = correctTranslationVector.at<float>(1, 0);
                    newFrameRelation->transformation_matrix.at<double>(2, 3) = correctTranslationVector.at<float>(2, 0);



                    //distance between cams == 1 -> because translation vector is already scaled well
                    newFrameRelation->distance_between_cams_in_cm = length;



                    newFrameRelation->transformation_matrix_reprojection_error = 1;
                    std::cout << "Transformation matrix is!! = \n" << newFrameRelation->transformation_matrix << std::endl;
                    //this->currFrameRelation = newFrameRelation;
                    GlobalParams::getInstance().addNewRelation(newFrameRelation);
                    this->customFrameProcedureCnt = -1;
                }
                else{
                    printf("It is nullptr\n");
                }

                

                ImGui::CloseCurrentPopup(); // Close the popup

            }
            else if(!isValid && ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup(); // End the popup
        }


        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 213, 0, 255));
        switch(this->customFrameProcedureCnt)
        {
            case 0:
                ImGui::SameLine(); ImGui::Text("Select origin point on 1st image"); break;
            case 1:
                ImGui::SameLine(); ImGui::Text("Select origin point on 2nd image"); break;
            case 2:
                ImGui::SameLine(); ImGui::Text("Select 1st axis point on 1st image"); break;
            case 3:
                ImGui::SameLine(); ImGui::Text("Select 1st axis point on 2nd image"); break;
            case 4:
                ImGui::SameLine(); ImGui::Text("Select 2nd axis point on 1st image"); break;
            case 5:
                ImGui::SameLine(); ImGui::Text("Select 2nd axis point on 2st image"); break;
            case 6:
                ImGui::SameLine(); ImGui::Text("Select 3rd axis point on 1st image"); break;
            case 7:
                ImGui::SameLine(); ImGui::Text("Select 3rd axis point on 2st image"); break;

        }
        ImGui::PopStyleColor();



        
        

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        int picture1X = io.MousePos.x - pos.x;
        int picture1Y = io.MousePos.y - pos.y;

        int picture2X = picture1X - (this->resolution[0]+8);
        int picture2Y = picture1Y;



        ImVec2 zoom1_T = ImVec2(((float)(picture1X))/this->resolution[0],((float)(picture1Y))/this->resolution[1]);
        zoom1_T.x = this->zoom1_A.x + zoom1_T.x * (this->zoom1_B.x - this->zoom1_A.x);
        zoom1_T.y = this->zoom1_A.y + zoom1_T.y * (this->zoom1_B.y - this->zoom1_A.y);

        ImVec2 zoom2_T = ImVec2(((float)(picture2X))/this->resolution[0],((float)(picture2Y))/this->resolution[1]);
        zoom2_T.x = this->zoom2_A.x + zoom2_T.x * (this->zoom2_B.x - this->zoom2_A.x);
        zoom2_T.y = this->zoom2_A.y + zoom2_T.y * (this->zoom2_B.y - this->zoom2_A.y);


        //Window mora biti hoveran, keyShift mor
        bool window1Hovered = false;
        bool window2Hovered = false;
        bool shiftHolded = false;
        bool shiftClicked = false;
        bool zoomWheelMoved = false;
        float zoomLevel1 = this->zoom1_B.x - this->zoom1_A.x;
        float zoomLevel2 = this->zoom2_B.x - this->zoom2_A.x;
       

        if((picture1X >= 0) && (picture1X <= this->resolution[0]) && (picture1Y >= 0) && (picture1Y <= this->resolution[1]))
        {
            window1Hovered = true;
        }
        else if((picture2X >= 0) && (picture2X <= this->resolution[0]) && (picture2Y >= 0) && (picture2Y <= this->resolution[1]))
        {
            window2Hovered = true;
        }
        //printf("Zoom2 = %.2f, %.2f, %d, %.2f, %.2f, %d\n", zoom1_T.x, zoom1_T.y, window1Hovered, zoom2_T.x, zoom2_T.y, window2Hovered);

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
        
        if ((window1Hovered || window2Hovered) && ImGui::IsKeyPressed(ImGuiKey_A) && this->calibrate_extrinsic_flag) {
            shouldAutoChessboardCalibrate = true;
        }

        if(shouldAutoChessboardCalibrate)
        {
            printf("SHould try to autochessboardcalibrate\n");
            cv::Size patternSize(5, 3); // 4 columns, 3 rows

            // Points for storing detected corners
            std::vector<cv::Point2f> corners1, corners2;

            // Detect chessboard corners in the first image
            bool found1 = cv::findChessboardCorners(this->firstLastMsg.first, patternSize, corners1, 
                                                    cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

            // Detect chessboard corners in the second image
            bool found2 = cv::findChessboardCorners(this->secondLastMsg.first, patternSize, corners2, 
                                                    cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

            if(found1 && found2)
            {
                for(int i = 0; i < corners1.size(); i++)
                {
                    this->pairList.push_back(std::make_pair(ImVec2(corners1[i].x, corners1[i].y), ImVec2(corners2[i].x, corners2[i].y)));
                }
                
            }


            shouldAutoChessboardCalibrate = false;
        }
        if((window1Hovered || window2Hovered) && shiftClicked) // freeze the image
        {
            freezeFrameFlag = true;
            //this->freezedFrame =  hsvFrame.clone();
            //printf("Freeze!\n");
        }

        if(window1Hovered && shiftHolded && zoomWheelMoved)
        {
            if ((ImGui::GetIO().MouseWheel > 0.0f) && (zoomLevel1 > 0.05)) { // zoom in critera

                //By how much will i zoom in/out -> every zoom in i zoom by tenth of previous value

                zoomLevel1 = zoomLevel1 * 0.9;

                //Recalculate new zoom level
                zoom1_A.x = zoom1_T.x - zoomLevel1 * ((zoom1_T.x - zoom1_A.x)/(zoom1_B.x - zoom1_A.x)); // zoom_A.x = zoom_T.x - zoom_T.x*zoomLevel;
                zoom1_B.x = zoom1_A.x + zoomLevel1;

                zoom1_A.y = zoom1_T.y - zoomLevel1 * ((zoom1_T.y - zoom1_A.y)/(zoom1_B.y - zoom1_A.y)); // zoom_A.y = zoom_T.y - zoom_T.y*zoomLevel;
                zoom1_B.y = zoom1_A.y + zoomLevel1;


                //printf("Scrool up, zoomLevel = %.2f\n", zoomLevel);
                //printf("zoom_T = %.2f, %.2f\n", zoom_T.x, zoom_T.y);
                //printf("zoom_A = %.2f, %.2f\n", zoom_A.x, zoom_A.y);
                //printf("zoom_B = %.2f, %.2f\n", zoom_B.x, zoom_B.y);

            } else { // zoom out criteria
                //printf("Scrool down\n");
                zoomLevel1 = zoomLevel1 * 1.1;
                if(zoomLevel1 > 1)
                {
                    zoomLevel1 = 1;
                }

                zoom1_A.x = zoom1_T.x - zoomLevel1 * ((zoom1_T.x - zoom1_A.x)/(zoom1_B.x - zoom1_A.x)); // zoom_A.x = zoom_T.x - zoom_T.x*zoomLevel;
                zoom1_B.x = zoom1_A.x + zoomLevel1;

                zoom1_A.y = zoom1_T.y - zoomLevel1 * ((zoom1_T.y - zoom1_A.y)/(zoom1_B.y - zoom1_A.y)); // zoom_A.y = zoom_T.y - zoom_T.y*zoomLevel;
                zoom1_B.y = zoom1_A.y + zoomLevel1;

                //provjeriti granice... i pomaknuti ih u skladu....
                if(zoom1_A.x < 0)
                {
                    zoom1_B.x += -zoom1_A.x;
                    zoom1_A.x = 0;
                }
                
                if(zoom1_B.x > 1)
                {
                    zoom1_A.x -= zoom1_B.x - 1;
                    zoom1_B.x = 1;
                }

                if(zoom1_A.y < 0)
                {
                    zoom1_B.y += -zoom1_A.y;
                    zoom1_A.y = 0;
                }
                if(zoom1_B.y > 1)
                {
                    zoom1_A.y -= zoom1_B.y - 1;
                    zoom1_B.y = 1;
                }

            }
        }
        
        else if(window2Hovered && shiftHolded && zoomWheelMoved)
        {
            if ((ImGui::GetIO().MouseWheel > 0.0f) && (zoomLevel2 > 0.05)) { // zoom in critera

                //By how much will i zoom in/out -> every zoom in i zoom by tenth of previous value

                zoomLevel2 = zoomLevel2 * 0.9;

                //Recalculate new zoom level
                zoom2_A.x = zoom2_T.x - zoomLevel2 * ((zoom2_T.x - zoom2_A.x)/(zoom2_B.x - zoom2_A.x)); // zoom_A.x = zoom_T.x - zoom_T.x*zoomLevel;
                zoom2_B.x = zoom2_A.x + zoomLevel2;

                zoom2_A.y = zoom2_T.y - zoomLevel2 * ((zoom2_T.y - zoom2_A.y)/(zoom2_B.y - zoom2_A.y)); // zoom_A.y = zoom_T.y - zoom_T.y*zoomLevel;
                zoom2_B.y = zoom2_A.y + zoomLevel2;


                //printf("Scrool up, zoomLevel = %.2f\n", zoomLevel);
                //printf("zoom_T = %.2f, %.2f\n", zoom_T.x, zoom_T.y);
                //printf("zoom_A = %.2f, %.2f\n", zoom_A.x, zoom_A.y);
                //printf("zoom_B = %.2f, %.2f\n", zoom_B.x, zoom_B.y);

            } else { // zoom out criteria
                //printf("Scrool down\n");
                zoomLevel2 = zoomLevel2 * 1.1;
                if(zoomLevel2 > 1)
                {
                    zoomLevel2 = 1;
                }

                zoom2_A.x = zoom2_T.x - zoomLevel2 * ((zoom2_T.x - zoom2_A.x)/(zoom2_B.x - zoom2_A.x)); // zoom_A.x = zoom_T.x - zoom_T.x*zoomLevel;
                zoom2_B.x = zoom2_A.x + zoomLevel2;

                zoom2_A.y = zoom2_T.y - zoomLevel2 * ((zoom2_T.y - zoom2_A.y)/(zoom2_B.y - zoom2_A.y)); // zoom_A.y = zoom_T.y - zoom_T.y*zoomLevel;
                zoom2_B.y = zoom2_A.y + zoomLevel2;

                //provjeriti granice... i pomaknuti ih u skladu....
                if(zoom2_A.x < 0)
                {
                    zoom2_B.x += -zoom2_A.x;
                    zoom2_A.x = 0;
                }
                
                if(zoom2_B.x > 1)
                {
                    zoom2_A.x -= zoom2_B.x - 1;
                    zoom2_B.x = 1;
                }

                if(zoom2_A.y < 0)
                {
                    zoom2_B.y += -zoom2_A.y;
                    zoom2_A.y = 0;
                }
                if(zoom2_B.y > 1)
                {
                    zoom2_A.y -= zoom2_B.y - 1;
                    zoom2_B.y = 1;
                }

            }
        }
        
        
        
        //Util::mat2Texture(this->firstCamMsg->data->first, this->firstCamMsg->data->second, this->texture1stCam);
        //ImGui::Image((ImTextureID)(intptr_t)this->texture1stCam, ImVec2(this->resolution[0], this->resolution[1]), this->zoom1_A, this->zoom1_B);
        
        //ImGui::Text("picture1 = %d, %d, picture2 = %d, %d\n", pictureX, pictureY, pictureX - 648, pictureY );
        
        

        //Now check if middle click is clicked and if it is move closes point.
        if(this->calibrate_extrinsic_flag)
        {
            float minDistance1 = 200;
            int closestPoint1 = -1;
            for(int i = 0; i < this->pairList.size(); i++)
            { 
                ImVec2 pointScaled =  pairList[i].first;
                pointScaled.x = pointScaled.x / this->resolution[0];
                pointScaled.y = pointScaled.y / this->resolution[1];

                float dist = sqrt(pow((pointScaled.x - zoom1_T.x), 2) + pow((pointScaled.y - zoom1_T.y), 2));
                if (dist < minDistance1) {
                    minDistance1 = dist;
                    closestPoint1 = i;
                }
            }

            if(window1Hovered && (closestPoint1 != -1) && this->customFrameProcedureCnt == -1)
            {
                this->closestPairIndex = closestPoint1;

                if(io.MouseDown[2])
                {
                    this->pairList[closestPoint1].first.x = zoom1_T.x * this->resolution[0];
                    this->pairList[closestPoint1].first.y = zoom1_T.y * this->resolution[1];
                }
                
            }


            float minDistance2 = 200;
            int closestPoint2 = -1;
            for(int i = 0; i < this->pairList.size(); i++)
            { 
                ImVec2 pointScaled =  pairList[i].second;
                pointScaled.x = pointScaled.x / this->resolution[0];
                pointScaled.y = pointScaled.y / this->resolution[1];

                float dist = sqrt(pow((pointScaled.x - zoom2_T.x), 2) + pow((pointScaled.y - zoom2_T.y), 2));
                if (dist < minDistance2) {
                    minDistance2 = dist;
                    closestPoint2 = i;
                }
            }


            if(window2Hovered && (closestPoint2 != -1))
            {
                //find closest value;
                this->closestPairIndex = closestPoint2;

                if(io.MouseDown[2] && this->customFrameProcedureCnt == -1)
                {
                    this->pairList[closestPoint2].second.x = zoom2_T.x * this->resolution[0];
                    this->pairList[closestPoint2].second.y = zoom2_T.y * this->resolution[1];
                }
                
            }

            //Now check if right click is clicked and add new point to pair, if pair is complete add to list of pairs
            if(window1Hovered && ImGui::IsMouseClicked(1)) 
            {
                pairNew.first.x = zoom1_T.x*this->resolution[0];
                pairNew.first.y = zoom1_T.y*this->resolution[1];
                printf("Right mouse clicked on frame 1, zoom1_T = %.4f, %.4f\n", pairNew.first.x, pairNew.first.y);

            }
            else if(window2Hovered && ImGui::IsMouseClicked(1))
            {
                pairNew.second.x = zoom2_T.x*this->resolution[0];
                pairNew.second.y = zoom2_T.y*this->resolution[1];
                printf("Right mouse clicked on frame 1, zoom1_T = %.4f, %.4f\n", pairNew.second.x, pairNew.second.y);
            }

            if((pairNew.first.x != 0) && (pairNew.second.x != 0))
            {
                this->pairList.push_back(pairNew);
                this->pairNew.first.x = 0;
                this->pairNew.first.y = 0;
                this->pairNew.second.x = 0;
                this->pairNew.second.y = 0;
            }
        }
        

        if((window1Hovered || window2Hovered) && ImGui::IsMouseClicked(3))
        {
            shouldBeColorView = !shouldBeColorView;
        }
        

        if(shouldBeColorView && window1Hovered && ImGui::IsKeyPressed(ImGuiKey_R))
        {
            this->hue1[0] = -1;
            this->hue1[1] = -1;
            this->sat1[0] = 255;
            this->sat1[1] = 0;
            this->val1[0] = 255;
            this->val1[1] = 0;

            cam1->colorThresholdParams.hue[0] = this->hue1[0];
            cam1->colorThresholdParams.hue[1] = this->hue1[1];
            cam1->colorThresholdParams.separation[0] = this->sat1[0];
            cam1->colorThresholdParams.separation[1] = this->sat1[1];
            cam1->colorThresholdParams.value[0] = this->val1[0];
            cam1->colorThresholdParams.value[1] = this->val1[1];
        }
        else if(shouldBeColorView && window2Hovered && ImGui::IsKeyPressed(ImGuiKey_R))
        {
            this->hue2[0] = -1;
            this->hue2[1] = -1;
            this->sat2[0] = 255;
            this->sat2[1] = 0;
            this->val2[0] = 255;
            this->val2[1] = 0;

            cam2->colorThresholdParams.hue[0] = this->hue2[0];
            cam2->colorThresholdParams.hue[1] = this->hue2[1];
            cam2->colorThresholdParams.separation[0] = this->sat2[0];
            cam2->colorThresholdParams.separation[1] = this->sat2[1];
            cam2->colorThresholdParams.value[0] = this->val2[0];
            cam2->colorThresholdParams.value[1] = this->val2[1];
        }

        //Check if should intrinsicaly calibrate..
        if(window1Hovered && ImGui::IsMouseClicked(4))
        {
            this->firstCamShouldIntrinsicalyCorrect = !this->firstCamShouldIntrinsicalyCorrect;
        }
        else if(window2Hovered && ImGui::IsMouseClicked(4))
        {
            this->secondCamShouldIntrinsicalyCorrect = !this->secondCamShouldIntrinsicalyCorrect;
        }


        ImVec2 imagePosLeft;
        ImVec2 imagePosRight;
        if(!this->firstLastMsg.first.empty())
        {

            

            //printf("pictureX = %d, pictureY = %d\n", pictureX, pictureY);
            
            //printf("Dosao %s, mat_1st = %d, mat_2nd = %d\n"
            //, this->firstCamMsg->camOrigin->frameNickName.c_str()
            //, this->firstCamMsg->data->first.size().area()
            //, this->firstCamMsg->data->second.size().area());
            //printf("Mat size = %d, %d\n", this->firstLastMsg.first.rows, this->firstLastMsg.second.rows);

            // Get the drawing list
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Get image position on the screen BEFORE calling ImGui::Image
            ImVec2 imagePos = ImGui::GetCursorScreenPos();
            imagePosLeft = imagePos;
            

            //If intrinsics enabled -> transform to intrinsic correct view
            
            //newFrame = tempImage;
            // Render the image
            cv::Mat tempImage = this->firstLastMsg.first.clone();
            cv::Mat colorFilter = this->firstLastMsg.second.clone();

            if(this->firstCamShouldIntrinsicalyCorrect)
            {
                cv::undistort(this->firstLastMsg.first, tempImage, cameraMatrix1, distCoeffs1);
            }




            
            {
                cv::Mat hsvFrame;
                cvtColor(tempImage, hsvFrame, cv::COLOR_BGR2HSV);

                if(window1Hovered && this->shouldBeColorView)
                {
                    //printf("Do something with pic1, mouse position = %.2f, %.2f\n",); // -> see what pixel is clicked & get coresponding color on Mat -> and update filter
                    //uzmi pixel koji
                    int dragXPicturePos = zoom1_T.x * this->resolution[0];
                    int dragYPicturePos = zoom1_T.y * this->resolution[1];

                    if((ImGui::IsMouseDown(1) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
                    {
                        cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(dragYPicturePos, dragXPicturePos);

                        int left_abs = Util::distLeft(this->hue1[0], hsv[0], 180);
                        int right_abs = Util::distRight(this->hue1[1], hsv[0], 180);
                        int range_abs = Util::distLeft(this->hue1[1], this->hue1[0], 180);
                        //printf("Left abs = %d, right_abs = %d, range_abs = %d[%d, %d]\n", left_abs, right_abs, range_abs, this->hue1[0], this->hue1[1]);


                        if((this->hue1[0] == -1 )|| (this->hue1[1] == -1)) // set new hsv
                        {
                            this->hue1[0] = hsv[0];
                            this->hue1[1] = hsv[0];
                        }



                        else if((left_abs <= right_abs) && (left_abs < (180 - range_abs)))
                        {
                            //printf("Dist left = %d\n", Util::distLeft(this->hue1[0], hsv[0], 180));
                            this->hue1[0] = hsv[0];
                        }
                        else if((right_abs < left_abs) && (right_abs < (180 - range_abs)))
                        {
                            //printf("Dist right = %d\n", Util::distRight(this->hue1[1], hsv[0], 180));
                            this->hue1[1] = hsv[0];
                        }

                        //if(hsv[0] < this->hue1[0]){
                        //    this->hue1[0] = hsv[0];
                        //}
                        //if(hsv[0] > this->hue1[1]){
                        //    this->hue1[1] = hsv[0];
                        //}

                        if(hsv[1] < this->sat1[0]){
                            this->sat1[0] = hsv[1];
                        }
                        if(hsv[1] > this->sat1[1]){
                            this->sat1[1] = hsv[1];
                        }

                        if(hsv[2] < this->val1[0]){
                            this->val1[0] = hsv[2];
                        }
                        if(hsv[2] > this->val1[1]){
                            this->val1[1] = hsv[2];
                        }

                        cam1->colorThresholdParams.hue[0] = this->hue1[0];
                        cam1->colorThresholdParams.hue[1] = this->hue1[1];
                        cam1->colorThresholdParams.separation[0] = this->sat1[0];
                        cam1->colorThresholdParams.separation[1] = this->sat1[1];
                        cam1->colorThresholdParams.value[0] = this->val1[0];
                        cam1->colorThresholdParams.value[1] = this->val1[1];

                    }
                }
                
                
                for(int x = 0; x < hsvFrame.cols; x++){
                    for(int y = 0; y < hsvFrame.rows; y++){
                        cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(y, x);
                        
                        
                        if(Util::isInRange(hsv[0], this->hue1[0], this->hue1[1], 180)
                        && hsv[1] >= this->sat1[0] && hsv[1] <= this->sat1[1]
                        && hsv[2] >= this->val1[0] && hsv[2] <= this->val1[1]){
                            colorFilter.at<u_char>(y,x) = 255;

                        }
                        else{
                            colorFilter.at<u_char>(y,x) = 70;
                        }
                        
                    }
                }
                
                
            }

            
            if(this->shouldBeColorView)
            {
                Util::mat2Texture(tempImage, colorFilter, this->texture1stCam);
            }
            else{
                Util::mat2Texture(tempImage, this->firstLastMsg.second, this->texture1stCam);
            }
            
            
            //Util::mat2Texture(tempImage, tempFilter, this->texture1stCam);
            ImGui::Image((ImTextureID)(intptr_t)this->texture1stCam, ImVec2(this->resolution[0], this->resolution[1]), this->zoom1_A, this->zoom1_B);

            
            if(window2Hovered && showEpipolarLines)
            {
                std::shared_ptr<cv::Point3f> epipolarLineParams = Util::getEpipolarLineCoefficiens(cam2, cv::Point2f(zoom2_T.x*this->resolution[0], zoom2_T.y*this->resolution[1]), cam1);
                if(epipolarLineParams != nullptr)
                {
                    //std::cout << "epipolarLineParams = " << *epipolarLineParams << std::endl;
                    

                    //ax + by + c = 0
                    float leftMarginYPoint = -epipolarLineParams->z/epipolarLineParams->y;
                    float topMarginXPoint = -epipolarLineParams->z/epipolarLineParams->x;
                    float rightMarginYPoint = (-epipolarLineParams->z - epipolarLineParams->x*this->resolution[0])/epipolarLineParams->y;
                    float bottomMarginXPoint = (-epipolarLineParams->z - epipolarLineParams->y*this->resolution[1])/epipolarLineParams->x;

                    cv::Point2f finalPoints[4];
                    int cnt = 0;

                    if((leftMarginYPoint >= 0) && (leftMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = 0;
                        finalPoints[cnt].y = leftMarginYPoint;
                        cnt++;
                    }
                    if((rightMarginYPoint >= 0) && (rightMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = this->resolution[0];
                        finalPoints[cnt].y = rightMarginYPoint;
                        cnt++;
                    }
                    if((topMarginXPoint >= 0) && (topMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = topMarginXPoint;
                        finalPoints[cnt].y = 0;
                        cnt++;
                    }
                    if((bottomMarginXPoint >= 0) && (bottomMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = bottomMarginXPoint;
                        finalPoints[cnt].y = this->resolution[1];
                        cnt++;
                    }

                    /*
                    printf("leftMarginYPoint = %.2f, topMarginXPoint = %.2f, rightMarginYPoint = %.2f, bottomMarginXPoint = %.2f\n"
                    , leftMarginYPoint, topMarginXPoint, rightMarginYPoint, bottomMarginXPoint);
                    
                    printf("Points = %d\n", cnt);
                    for(int i = 0; i < cnt; i++)
                    {
                        std::cout << finalPoints[i] << std::endl;
                    }
                    */

                    if(cnt == 2)
                    {
                        //drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        //printf("Adding line\n");
                        //Util::drawStrip(drawList, )
                        drawList->AddLine(ImVec2(imagePos.x + finalPoints[0].x, imagePos.y + finalPoints[0].y), ImVec2(imagePos.x + finalPoints[1].x, imagePos.y + finalPoints[1].y), IM_COL32(255, 255, 255, 255), 4.0f);
                    }
                    
                    //ax + ay = z, if 
                }
            }


            if(this->customFrameProcedureCnt == 0)
            {
                //check if it is clicked on first frame somewhere
                if(window1Hovered && ImGui::IsMouseClicked(1)) 
                {
                    this->custom_frame_origin_left.x = zoom1_T.x*this->resolution[0];
                    this->custom_frame_origin_left.y = zoom1_T.y*this->resolution[1];
                    this->customFrameProcedureCnt++;
                }
            }
            else if(this->customFrameProcedureCnt == 2)
            {
                //check if it is clicked on first frame somewhere
                if(window1Hovered && ImGui::IsMouseClicked(1)) 
                {
                    this->custom_frame_point1_left.x = zoom1_T.x*this->resolution[0];
                    this->custom_frame_point1_left.y = zoom1_T.y*this->resolution[1];
                    this->customFrameProcedureCnt++;
                }
            }
            else if(this->customFrameProcedureCnt == 4)
            {
                if(window1Hovered && ImGui::IsMouseClicked(1)) 
                {
                    this->custom_frame_point2_left.x = zoom1_T.x*this->resolution[0];
                    this->custom_frame_point2_left.y = zoom1_T.y*this->resolution[1];
                    this->customFrameProcedureCnt++;
                    
                    //Just remember this point as well
                    
                                        /*
                    //Calculate second axis...
                    std::vector<cv::Point2f> image_points = {secondPointAxisLeft};
                    cv::Mat undistorted_points;
                    cv::undistortPoints(image_points, undistorted_points, cam1->intrinsicParams.intrinsicMatrix, cam1->intrinsicParams.distortionCoef);

                    // Since undistorted_point is already normalized, you can directly use it:
                    cv::Point2f undistorted_point = undistorted_points.at<cv::Point2f>(0);

                    // The normalized coordinates are already scaled in the camera's intrinsic frame:
                    cv::Vec3f ray_direction(undistorted_point.x, undistorted_point.y, 1.0f);

                    // ray_direction(cam2) : [-0.189805, -0.11608, 1]
                    std::cout << "ray_direction("<< cam1->frameNickName <<") : " << ray_direction  << std::endl;
                    float r_dx = ray_direction[0];
                    float r_dy = ray_direction[1];
                    float r_dz = ray_direction[2];


                    //origin_left(cam2) : [-13.1398, 4.79123, 59.7908]
                    std::cout << "origin_left("<< cam1->frameNickName <<") : " << this->custom_frame_translationVector  << std::endl;
                    float x0 = this->custom_frame_translationVector.x;
                    float y0 = this->custom_frame_translationVector.y;
                    float z0 = this->custom_frame_translationVector.z;


                    //axis1_left(cam2) : [0.92851436;
                    //                    0.0841524;
                    //                    0.36163425]
                    std::cout << "plane normal("<< cam1->frameNickName <<") : " << this->custom_frame_rotationMatrix.col(0)  << std::endl;
                    float p_dx = this->custom_frame_rotationMatrix.at<float>(0, 0);
                    float p_dy = this->custom_frame_rotationMatrix.at<float>(2, 0);
                    float p_dz = this->custom_frame_rotationMatrix.at<float>(3, 0);


                    // Calculate the intersection parameter t
                    float denominator = p_dx * r_dx + p_dy * r_dy + p_dz * r_dz;

                    if (std::abs(denominator) < 1e-6) {
                        std::cerr << "The ray is parallel to the plane and does not intersect." << std::endl;
                        return;
                    }

                    float numerator = p_dx * x0 + p_dy * y0 + p_dz * z0;
                    float t = numerator / denominator;

                    // Compute intersection point
                    float x_intersect = x0 - t * r_dx;
                    float y_intersect = y0 - t * r_dy;
                    float z_intersect = z0 - t * r_dz;

                    // Output the intersection point
                    std::cout << "Intersection Point: (" << x_intersect << ", " << y_intersect << ", " << z_intersect << ")" << std::endl;
                    */
                    //this->customFrameProcedureCnt++;
                }
            }
            else if(this->customFrameProcedureCnt == 6)
            {
                if((window1Hovered || window2Hovered) && ImGui::IsMouseClicked(1)) 
                {
                    this->custom_frame_rotationMatrix.at<float>(0, 2) = -this->custom_frame_rotationMatrix.at<float>(0, 2);
                    this->custom_frame_rotationMatrix.at<float>(1, 2) = -this->custom_frame_rotationMatrix.at<float>(1, 2);
                    this->custom_frame_rotationMatrix.at<float>(2, 2) = -this->custom_frame_rotationMatrix.at<float>(2, 2);
                    //std::cout << "this->custom_frame_rotationMatrix" << this->custom_frame_rotationMatrix << std::endl;
                }
            }


            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> contoursHierarchy;
            cv::Mat binaryImage;
            cv::threshold(colorFilter, binaryImage, 254, 1, cv::THRESH_BINARY);
            cv::findContours(binaryImage, contours, contoursHierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
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

            std::vector<std::pair<cv::Point2f, float>> circles;
            float maxRadius = 0;
            int maxRadiusIndex = -1;
            for(int i = 0; i < myHierarchy.size(); i++){
                cv::Point2f newCenter;
                float newRadius;

                int parentId = myHierarchy[i].first;
                std::vector<cv::Point> parentContour = contours[parentId];
                cv::minEnclosingCircle(parentContour, newCenter, newRadius);

                if(newRadius > maxRadius)
                {
                    maxRadius = newRadius;
                    maxRadiusIndex = i;
                }
                circles.push_back(std::make_pair(newCenter, newRadius));
            }    
            
            if(maxRadiusIndex >= 0)
            {
                //printf("Found circles = %d, max_radius = %.2f, x = %.2f, %.2f, %d\n", circles.size(), maxRadius, circles[maxRadiusIndex].first.x, circles[maxRadiusIndex].first.y, maxRadiusIndex);

                std::vector<cv::Point2f> distorted_points;
                std::vector<cv::Point2f> undistorted_points;
                ImVec2 pointScaled;
                pointScaled.x = circles[maxRadiusIndex].first.x;
                pointScaled.y = circles[maxRadiusIndex].first.y;

                this->cam1_2D.x = pointScaled.x;
                this->cam1_2D.y = pointScaled.y;
                this->cam1_2D_updated = true;
                //std::cout << "this->cam1_2D" << this->cam1_2D <<std::endl;

                if(this->firstCamShouldIntrinsicalyCorrect)
                {
                    distorted_points.clear();
                    distorted_points.push_back(cv::Point2d(pointScaled.x, pointScaled.y));
                    cv::undistortPoints(distorted_points, undistorted_points, cameraMatrix1, distCoeffs1);
                    cv::Point2d normPoint = undistorted_points.back();
                    pointScaled.x = (normPoint.x * cameraMatrix1.at<double>(0, 0) + cameraMatrix1.at<double>(0, 2)) / (float)this->resolution[0]; // Normalize to image width
                    pointScaled.y = (normPoint.y * cameraMatrix1.at<double>(1, 1) + cameraMatrix1.at<double>(1, 2)) / (float)this->resolution[1]; // Normalize to image height
                
                }
                else{
                    pointScaled.x = pointScaled.x / this->resolution[0];
                    pointScaled.y = pointScaled.y / this->resolution[1];
                }

                if(this->shouldBeColorView)
                {
                    if((pointScaled.x > this->zoom1_A.x) && (pointScaled.x < this->zoom1_B.x)
                        && (pointScaled.y > this->zoom1_A.y) && (pointScaled.y < this->zoom1_B.y)) // draw condition
                    {
                        float x_pixel = ((pointScaled.x - this->zoom1_A.x) / (this->zoom1_B.x - this->zoom1_A.x))*this->resolution[0];
                        float y_pixel = ((pointScaled.y - this->zoom1_A.y) / (this->zoom1_B.y - this->zoom1_A.y))*this->resolution[1];
                        ImVec2 circlePos = ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel);





                        // Your hue, saturation, and value inputs
                        //float hue = (this->hue1[0] + (this->hue1[1] - this->hue1[0]) / 2)*1.41176;
                        //float sat = (this->sat1[0] + (this->sat1[1] - this->sat1[0]) / 2)*1.41176;
                        //float val = (this->val1[0] + (this->val1[1] - this->val1[0]) / 2) * 1.41176;

                        // Convert HSV to RGB using ImGui
                        float r_f = 0.0f, g_f = 0.0f, b_f = 0.0f;
                        // Normalize the input ranges to 0.0 to 1.0
                        float h = (Util::getAvgValue(this->hue1[0], this->hue1[1], 180)) / 180.0f;
                        float s = (this->sat1[0] + (this->sat1[1] - this->sat1[0]) / 2) / 255.0f;
                        float v = (this->val1[0] + (this->val1[1] - this->val1[0]) / 2) / 255.0f;
                        ImGui::ColorConvertHSVtoRGB(h, 1.0f, 1.0f, r_f, g_f, b_f);
                        int r = static_cast<int>(r_f * 255);
                        int g = static_cast<int>(g_f * 255);
                        int b = static_cast<int>(b_f * 255);

                        // Print the result
                        //printf("Red = %d, Green = %d, Blue = %d, h = %.2f, s = %.2f\n", r, g, b);
                        //printf("h = %.2f[%d, %d], s = %.2f[%d], v = %.2f[%d]\n", h, this->hue1[0], this->hue1[1],
                        // s, this->sat1[0] + (this->sat1[1] - this->sat1[0]) / 2,
                        // v, this->val1[0] + (this->val1[1] - this->val1[0]) / 2);

                        
                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        float scalingFactor = 1.0f/(zoom1_B.x-zoom1_A.x);
                        drawList->AddCircle(circlePos, maxRadius*scalingFactor, IM_COL32(r, g, b, 255), 50, 4.0f);
                        float addedTrigonometry = (sqrt(2)/2)*maxRadius*scalingFactor;
                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y - addedTrigonometry), IM_COL32(r, g, b, 120), 2.0f);

                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y - addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y + addedTrigonometry), IM_COL32(r, g, b, 120), 2.0f);


                        addedTrigonometry = (sqrt(2)/2)*(maxRadius > 10 ? 10 : maxRadius);
                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y - addedTrigonometry), IM_COL32(r, g, b, 255), 4.0f);

                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y - addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y + addedTrigonometry), IM_COL32(r, g, b, 255), 4.0f);
                    }
                }
                
            }
            
            //Draw all calibratin circles
            if(!this->shouldBeColorView && this->calibrate_extrinsic_flag)
            {
                std::vector<cv::Point2f> distorted_points;
                std::vector<cv::Point2f> undistorted_points;
                for(int i = 0; i < this->pairList.size(); i++)
                {

                    ImVec2 pointScaled =  pairList[i].first;
                    if(this->firstCamShouldIntrinsicalyCorrect)
                    {
                        distorted_points.clear();
                        distorted_points.push_back(cv::Point2d(pairList[i].first.x, pairList[i].first.y));
                        cv::undistortPoints(distorted_points, undistorted_points, cameraMatrix1, distCoeffs1);
                        cv::Point2d normPoint = undistorted_points.back();
                        pointScaled.x = (normPoint.x * cameraMatrix1.at<double>(0, 0) + cameraMatrix1.at<double>(0, 2)) / (float)this->resolution[0]; // Normalize to image width
                        pointScaled.y = (normPoint.y * cameraMatrix1.at<double>(1, 1) + cameraMatrix1.at<double>(1, 2)) / (float)this->resolution[1]; // Normalize to image height
                    
                    }
                    else{
                        pointScaled.x = pointScaled.x / this->resolution[0];
                        pointScaled.y = pointScaled.y / this->resolution[1];
                    }


                    if((pointScaled.x > this->zoom1_A.x) && (pointScaled.x < this->zoom1_B.x)
                    && (pointScaled.y > this->zoom1_A.y) && (pointScaled.y < this->zoom1_B.y)) // draw condition
                    {
                        float x_pixel = ((pointScaled.x - this->zoom1_A.x) / (this->zoom1_B.x - this->zoom1_A.x))*this->resolution[0];
                        float y_pixel = ((pointScaled.y - this->zoom1_A.y) / (this->zoom1_B.y - this->zoom1_A.y))*this->resolution[1];
                        ImVec2 circlePos = ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel);


                        // Draw a red circle
                        if(i == this->closestPairIndex)
                        {
                            drawList->AddCircle(circlePos, 10.0f, IM_COL32(255, 255, 0, 255), 12, 4.0f);
                        }
                        else
                        {
                            drawList->AddCircle(circlePos, 10.0f, IM_COL32(255, 255, 0, 100), 12, 4.0f);
                        }
                    }
                }
            }
            

        }
        if(!this->secondLastMsg.first.empty())
        {
            ImGui::SameLine();


            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Get image position on the screen BEFORE calling ImGui::Image
            ImVec2 imagePos = ImGui::GetCursorScreenPos();
            imagePosRight = imagePos;
            

            cv::Mat tempImage = this->secondLastMsg.first.clone();
            cv::Mat colorFilter = this->secondLastMsg.second.clone();
            if(this->secondCamShouldIntrinsicalyCorrect)
            {
                cv::undistort(this->secondLastMsg.first, tempImage, cameraMatrix2, distCoeffs2);
            }




            
            {
                cv::Mat hsvFrame;
                cvtColor(tempImage, hsvFrame, cv::COLOR_BGR2HSV);
                if(window2Hovered && this->shouldBeColorView)
                {
                    //printf("Do something with pic1, mouse position = %.2f, %.2f\n",); // -> see what pixel is clicked & get coresponding color on Mat -> and update filter
                    //uzmi pixel koji
                    int dragXPicturePos = zoom2_T.x * this->resolution[0];
                    int dragYPicturePos = zoom2_T.y * this->resolution[1];

                    if((ImGui::IsMouseDown(1) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
                    {
                        cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(dragYPicturePos, dragXPicturePos);

                        //Ako je 
                        
                        int left_abs = Util::distLeft(this->hue2[0], hsv[0], 180);
                        int right_abs = Util::distRight(this->hue2[1], hsv[0], 180);
                        int range_abs = Util::distLeft(this->hue2[1], this->hue2[0], 180);
                        //printf("Left abs = %d, right_abs = %d, range_abs = %d[%d, %d]\n", left_abs, right_abs, range_abs, this->hue1[0], this->hue1[1]);


                        if((this->hue2[0] == -1 )|| (this->hue2[1] == -1)) // set new hsv
                        {
                            this->hue2[0] = hsv[0];
                            this->hue2[1] = hsv[0];
                        }



                        else if((left_abs <= right_abs) && (left_abs < (180 - range_abs)))
                        {
                            //printf("Dist left = %d\n", Util::distLeft(this->hue1[0], hsv[0], 180));
                            this->hue2[0] = hsv[0];
                        }
                        else if((right_abs < left_abs) && (right_abs < (180 - range_abs)))
                        {
                            //printf("Dist right = %d\n", Util::distRight(this->hue1[1], hsv[0], 180));
                            this->hue2[1] = hsv[0];
                        }


                        //if(hsv[0] < this->hue2[0]){
                        //    this->hue2[0] = hsv[0];
                        //}
                        //if(hsv[0] > this->hue2[1]){
                        //    this->hue2[1] = hsv[0];
                        //}

                        if(hsv[1] < this->sat2[0]){
                            this->sat2[0] = hsv[1];
                        }
                        if(hsv[1] > this->sat2[1]){
                            this->sat2[1] = hsv[1];
                        }

                        if(hsv[2] < this->val2[0]){
                            this->val2[0] = hsv[2];
                        }
                        if(hsv[2] > this->val2[1]){
                            this->val2[1] = hsv[2];
                        }

                        cam2->colorThresholdParams.hue[0] = this->hue2[0];
                        cam2->colorThresholdParams.hue[1] = this->hue2[1];
                        cam2->colorThresholdParams.separation[0] = this->sat2[0];
                        cam2->colorThresholdParams.separation[1] = this->sat2[1];
                        cam2->colorThresholdParams.value[0] = this->val2[0];
                        cam2->colorThresholdParams.value[1] = this->val2[1];
                    }
                }
                
                
                for(int x = 0; x < hsvFrame.cols; x++){
                    for(int y = 0; y < hsvFrame.rows; y++){
                        cv::Vec3b hsv = hsvFrame.at<cv::Vec3b>(y, x);
                        
                        
                        if(Util::isInRange(hsv[0], this->hue2[0], this->hue2[1], 180)
                        && hsv[1] >= this->sat2[0] && hsv[1] <= this->sat2[1]
                        && hsv[2] >= this->val2[0] && hsv[2] <= this->val2[1]){
                            colorFilter.at<u_char>(y,x) = 255;

                        }
                        else{
                            colorFilter.at<u_char>(y,x) = 70;
                        }
                        
                    }
                }
            }

            
            if(this->shouldBeColorView)
            {
                Util::mat2Texture(tempImage, colorFilter, this->texture2ndCam);
            }
            else
            {
                Util::mat2Texture(tempImage, this->secondLastMsg.second, this->texture2ndCam);
            }
            
           //Util::mat2Texture(tempImage, tempFilter, this->texture2ndCam);
            ImGui::Image((ImTextureID)(intptr_t)this->texture2ndCam, ImVec2(this->resolution[0], this->resolution[1]), this->zoom2_A, this->zoom2_B);

            if(window1Hovered && showEpipolarLines)
            {
                std::shared_ptr<cv::Point3f> epipolarLineParams = Util::getEpipolarLineCoefficiens(cam1, cv::Point2f(zoom1_T.x*this->resolution[0], zoom1_T.y*this->resolution[1]), cam2);
                if(epipolarLineParams != nullptr)
                {
                    //std::cout << "epipolarLineParams = " << *epipolarLineParams << std::endl;
                    


                    float leftMarginYPoint = -epipolarLineParams->z/epipolarLineParams->y;
                    float topMarginXPoint = -epipolarLineParams->z/epipolarLineParams->x;
                    float rightMarginYPoint = (-epipolarLineParams->z - epipolarLineParams->x*this->resolution[0])/epipolarLineParams->y;
                    float bottomMarginXPoint = (-epipolarLineParams->z - epipolarLineParams->y*this->resolution[1])/epipolarLineParams->x;

                    cv::Point2f finalPoints[4];
                    int cnt = 0;

                    if((leftMarginYPoint >= 0) && (leftMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = 0;
                        finalPoints[cnt].y = leftMarginYPoint;
                        cnt++;
                    }
                    if((rightMarginYPoint >= 0) && (rightMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = this->resolution[0];
                        finalPoints[cnt].y = rightMarginYPoint;
                        cnt++;
                    }
                    if((topMarginXPoint >= 0) && (topMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = topMarginXPoint;
                        finalPoints[cnt].y = 0;
                        cnt++;
                    }
                    if((bottomMarginXPoint >= 0) && (bottomMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = bottomMarginXPoint;
                        finalPoints[cnt].y = this->resolution[1];
                        cnt++;
                    }

                    /*
                    printf("leftMarginYPoint = %.2f, topMarginXPoint = %.2f, rightMarginYPoint = %.2f, bottomMarginXPoint = %.2f\n"
                    , leftMarginYPoint, topMarginXPoint, rightMarginYPoint, bottomMarginXPoint);
                    
                    printf("Points = %d\n", cnt);
                    for(int i = 0; i < cnt; i++)
                    {
                        std::cout << finalPoints[i] << std::endl;
                    }
                    */
                    

                    if(cnt == 2)
                    {
                        //drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        //printf("Adding line\n");
                        drawList->AddLine(ImVec2(imagePos.x + finalPoints[0].x, imagePos.y + finalPoints[0].y), ImVec2(imagePos.x + finalPoints[1].x, imagePos.y + finalPoints[1].y), IM_COL32(255, 255, 255, 255), 4.0f);
                    }
                    
                    //ax + ay = z, if 
                }
            }


            if(this->customFrameProcedureCnt == 1)
            {
                //show epipolar line of first
                std::shared_ptr<cv::Point3f> epipolarLineParams = Util::getEpipolarLineCoefficiens(cam1, cv::Point2f(this->custom_frame_origin_left.x, this->custom_frame_origin_left.y), cam2);
                if(epipolarLineParams != nullptr)
                {
                    //std::cout << "epipolarLineParams = " << *epipolarLineParams << std::endl;
                    

                    //ax + by + c = 0
                    float leftMarginYPoint = -epipolarLineParams->z/epipolarLineParams->y;
                    float topMarginXPoint = -epipolarLineParams->z/epipolarLineParams->x;
                    float rightMarginYPoint = (-epipolarLineParams->z - epipolarLineParams->x*this->resolution[0])/epipolarLineParams->y;
                    float bottomMarginXPoint = (-epipolarLineParams->z - epipolarLineParams->y*this->resolution[1])/epipolarLineParams->x;

                    cv::Point2f finalPoints[4];
                    int cnt = 0;

                    if((leftMarginYPoint >= 0) && (leftMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = 0;
                        finalPoints[cnt].y = leftMarginYPoint;
                        cnt++;
                    }
                    if((rightMarginYPoint >= 0) && (rightMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = this->resolution[0];
                        finalPoints[cnt].y = rightMarginYPoint;
                        cnt++;
                    }
                    if((topMarginXPoint >= 0) && (topMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = topMarginXPoint;
                        finalPoints[cnt].y = 0;
                        cnt++;
                    }
                    if((bottomMarginXPoint >= 0) && (bottomMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = bottomMarginXPoint;
                        finalPoints[cnt].y = this->resolution[1];
                        cnt++;
                    }

                    if(cnt == 2)
                    {
                        //drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        //printf("Adding line\n");
                        


                        //drawList->AddLine(ImVec2(imagePos.x + finalPoints[0].x, imagePos.y + finalPoints[0].y), ImVec2(imagePos.x + finalPoints[1].x, imagePos.y + finalPoints[1].y), IM_COL32(255, 0, 0, 255), 4.0f);
                        

                        if(window2Hovered)
                        {
                            //printf("Zoom2.x = %.2f, Zoom2.y = %.2f\n", zoom2_T.x, zoom2_T.y);
                        
                            
                            cv::Point2f closestPoint = Util::closestPointOnLine(epipolarLineParams->x, epipolarLineParams->y, epipolarLineParams->z, cv::Point2f(zoom2_T.x*this->resolution[0], zoom2_T.y*this->resolution[1]));
                            //printf("closestPoint = %.2f, %.2f\n", closestPoint.x, closestPoint.y);
                            float x_pixel = ((closestPoint.x/(float)this->resolution[0] - this->zoom2_A.x) / (this->zoom2_B.x - this->zoom2_A.x))*this->resolution[0];
                            float y_pixel = ((closestPoint.y/(float)this->resolution[1] - this->zoom2_A.y) / (this->zoom2_B.y - this->zoom2_A.y))*this->resolution[1];
                            if(x_pixel >= 0 && x_pixel <= this->resolution[0] && y_pixel >= 0 && y_pixel <= this->resolution[1])
                            {
                                drawList->AddCircle(ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);

                                if(ImGui::IsMouseClicked(1)) // create translation vector
                                {
                                    // I have both points! -> i can triangulate and create tranaslation vector
                                    std::shared_ptr<cv::Point3f> translation = Util::triangulate(cam1, this->custom_frame_origin_left, cam2, closestPoint, cam1);

                                    


                                    if(translation != nullptr)
                                    {
                                        std::cout << "Translation:" << *translation << std::endl;
                                        this->custom_frame_translationVector = *translation;
                                        this->customFrameProcedureCnt++;
                                    }
                                }
                            }
                            
                            
                        }
                    }
                    
                    //ax + ay = z, if 
                }
            }
            else if(this->customFrameProcedureCnt == 3)
            {
                //show epipolar line of first
                std::shared_ptr<cv::Point3f> epipolarLineParams = Util::getEpipolarLineCoefficiens(cam1, cv::Point2f(this->custom_frame_point1_left.x, this->custom_frame_point1_left.y), cam2);
                if(epipolarLineParams != nullptr)
                {
                    //std::cout << "epipolarLineParams = " << *epipolarLineParams << std::endl;
                    

                    //ax + by + c = 0
                    float leftMarginYPoint = -epipolarLineParams->z/epipolarLineParams->y;
                    float topMarginXPoint = -epipolarLineParams->z/epipolarLineParams->x;
                    float rightMarginYPoint = (-epipolarLineParams->z - epipolarLineParams->x*this->resolution[0])/epipolarLineParams->y;
                    float bottomMarginXPoint = (-epipolarLineParams->z - epipolarLineParams->y*this->resolution[1])/epipolarLineParams->x;

                    cv::Point2f finalPoints[4];
                    int cnt = 0;

                    if((leftMarginYPoint >= 0) && (leftMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = 0;
                        finalPoints[cnt].y = leftMarginYPoint;
                        cnt++;
                    }
                    if((rightMarginYPoint >= 0) && (rightMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = this->resolution[0];
                        finalPoints[cnt].y = rightMarginYPoint;
                        cnt++;
                    }
                    if((topMarginXPoint >= 0) && (topMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = topMarginXPoint;
                        finalPoints[cnt].y = 0;
                        cnt++;
                    }
                    if((bottomMarginXPoint >= 0) && (bottomMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = bottomMarginXPoint;
                        finalPoints[cnt].y = this->resolution[1];
                        cnt++;
                    }

                    if(cnt == 2)
                    {
                        //drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        //printf("Adding line\n");
                        


                        //drawList->AddLine(ImVec2(imagePos.x + finalPoints[0].x, imagePos.y + finalPoints[0].y), ImVec2(imagePos.x + finalPoints[1].x, imagePos.y + finalPoints[1].y), IM_COL32(255, 0, 0, 255), 4.0f);
                        

                        if(window2Hovered)
                        {
                            //printf("Zoom2.x = %.2f, Zoom2.y = %.2f\n", zoom2_T.x, zoom2_T.y);
                        
                            
                            cv::Point2f closestPoint = Util::closestPointOnLine(epipolarLineParams->x, epipolarLineParams->y, epipolarLineParams->z, cv::Point2f(zoom2_T.x*this->resolution[0], zoom2_T.y*this->resolution[1]));
                            //printf("closestPoint = %.2f, %.2f\n", closestPoint.x, closestPoint.y);
                            float x_pixel = ((closestPoint.x/(float)this->resolution[0] - this->zoom2_A.x) / (this->zoom2_B.x - this->zoom2_A.x))*this->resolution[0];
                            float y_pixel = ((closestPoint.y/(float)this->resolution[1] - this->zoom2_A.y) / (this->zoom2_B.y - this->zoom2_A.y))*this->resolution[1];
                            if(x_pixel >= 0 && x_pixel <= this->resolution[0] && y_pixel >= 0 && y_pixel <= this->resolution[1])
                            {
                                drawList->AddCircle(ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);

                                if(ImGui::IsMouseClicked(1)) // create translation vector
                                {
                                    // I have both points! -> i can triangulate and create tranaslation vector
                                    std::shared_ptr<cv::Point3f> cam1_firstAxisEnd = Util::triangulate(cam1, this->custom_frame_point1_left, cam2, closestPoint, cam1);
                                    //this is vector that goes from cam1 -> first axis end.
                                    //Now convert it to first axis start -> first axis end
                                    

                                    if(cam1_firstAxisEnd != nullptr)
                                    {
                                        cv::Point3f origin_firstAxisEnd = cv::Point3f((*cam1_firstAxisEnd).x - this->custom_frame_translationVector.x
                                                                                    , (*cam1_firstAxisEnd).y - this->custom_frame_translationVector.y
                                                                                    , (*cam1_firstAxisEnd).z - this->custom_frame_translationVector.z);
                                        cv::Mat normalizedColum = Util::normalizeVector3D(origin_firstAxisEnd);
                                        //This is practicaly 

                                        this->custom_frame_rotationMatrix.at<float>(0, 0) = normalizedColum.at<float>(0, 0);
                                        this->custom_frame_rotationMatrix.at<float>(1, 0) = normalizedColum.at<float>(1, 0);
                                        this->custom_frame_rotationMatrix.at<float>(2, 0) = normalizedColum.at<float>(2, 0);

                                        this->customFrameProcedureCnt++;
                                    }
                                }
                            }
                            
                            
                        }
                    }
                    
                    //ax + ay = z, if 
                }
            }
            else if(this->customFrameProcedureCnt == 5)
            {
                std::shared_ptr<cv::Point3f> epipolarLineParams = Util::getEpipolarLineCoefficiens(cam1, cv::Point2f(this->custom_frame_point2_left.x, this->custom_frame_point2_left.y), cam2);
                if(epipolarLineParams != nullptr)
                {
                    //std::cout << "epipolarLineParams = " << *epipolarLineParams << std::endl;
                    

                    //ax + by + c = 0
                    float leftMarginYPoint = -epipolarLineParams->z/epipolarLineParams->y;
                    float topMarginXPoint = -epipolarLineParams->z/epipolarLineParams->x;
                    float rightMarginYPoint = (-epipolarLineParams->z - epipolarLineParams->x*this->resolution[0])/epipolarLineParams->y;
                    float bottomMarginXPoint = (-epipolarLineParams->z - epipolarLineParams->y*this->resolution[1])/epipolarLineParams->x;

                    cv::Point2f finalPoints[4];
                    int cnt = 0;

                    if((leftMarginYPoint >= 0) && (leftMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = 0;
                        finalPoints[cnt].y = leftMarginYPoint;
                        cnt++;
                    }
                    if((rightMarginYPoint >= 0) && (rightMarginYPoint <= this->resolution[1]))
                    {
                        finalPoints[cnt].x = this->resolution[0];
                        finalPoints[cnt].y = rightMarginYPoint;
                        cnt++;
                    }
                    if((topMarginXPoint >= 0) && (topMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = topMarginXPoint;
                        finalPoints[cnt].y = 0;
                        cnt++;
                    }
                    if((bottomMarginXPoint >= 0) && (bottomMarginXPoint <= this->resolution[0]))
                    {
                        finalPoints[cnt].x = bottomMarginXPoint;
                        finalPoints[cnt].y = this->resolution[1];
                        cnt++;
                    }

                    if(cnt == 2)
                    {
                        //drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        //printf("Adding line\n");
                        


                        //drawList->AddLine(ImVec2(imagePos.x + finalPoints[0].x, imagePos.y + finalPoints[0].y), ImVec2(imagePos.x + finalPoints[1].x, imagePos.y + finalPoints[1].y), IM_COL32(255, 0, 0, 255), 4.0f);
                        

                        if(window2Hovered)
                        {
                            //printf("Zoom2.x = %.2f, Zoom2.y = %.2f\n", zoom2_T.x, zoom2_T.y);
                        
                            
                            cv::Point2f closestPoint = Util::closestPointOnLine(epipolarLineParams->x, epipolarLineParams->y, epipolarLineParams->z, cv::Point2f(zoom2_T.x*this->resolution[0], zoom2_T.y*this->resolution[1]));
                            //printf("closestPoint = %.2f, %.2f\n", closestPoint.x, closestPoint.y);
                            float x_pixel = ((closestPoint.x/(float)this->resolution[0] - this->zoom2_A.x) / (this->zoom2_B.x - this->zoom2_A.x))*this->resolution[0];
                            float y_pixel = ((closestPoint.y/(float)this->resolution[1] - this->zoom2_A.y) / (this->zoom2_B.y - this->zoom2_A.y))*this->resolution[1];
                            if(x_pixel >= 0 && x_pixel <= this->resolution[0] && y_pixel >= 0 && y_pixel <= this->resolution[1])
                            {
                                drawList->AddCircle(ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);

                                if(ImGui::IsMouseClicked(1)) // create translation vector
                                {
                                    // I have both points! -> i can triangulate and create tranaslation vector
                                    std::shared_ptr<cv::Point3f> cam1_secondAxisEnd = Util::triangulate(cam1, this->custom_frame_point2_left, cam2, closestPoint, cam1);
                                    //this is vector that goes from cam1 -> first axis end.
                                    //Now convert it to first axis start -> first axis end
                                    

                                    if(cam1_secondAxisEnd != nullptr)
                                    {
                                        std::cout << "origin_left("<< cam1->frameNickName <<") : " << this->custom_frame_translationVector  << std::endl;
                                        float x0 = this->custom_frame_translationVector.x;
                                        float y0 = this->custom_frame_translationVector.y;
                                        float z0 = this->custom_frame_translationVector.z;


                                        std::cout << "plane normal("<< cam1->frameNickName <<") : " << this->custom_frame_rotationMatrix.col(0)  << std::endl;
                                        float p_dx = this->custom_frame_rotationMatrix.at<float>(0, 0);
                                        float p_dy = this->custom_frame_rotationMatrix.at<float>(1, 0);
                                        float p_dz = this->custom_frame_rotationMatrix.at<float>(2, 0);


                                        cv::Point3f secondAxisFinal = Util::findClosestPointToPlane(p_dx, p_dy, p_dz, x0, y0, z0, cam1_secondAxisEnd->x, cam1_secondAxisEnd->y, cam1_secondAxisEnd->z);
                                        
                                        std::cout << "secondAxisFinal: " << secondAxisFinal << std::endl;


                                        //convert to vector & normalize

                                        cv::Point3f origin_firstAxisEnd = cv::Point3f(secondAxisFinal.x - this->custom_frame_translationVector.x
                                                                                    , secondAxisFinal.y - this->custom_frame_translationVector.y
                                                                                    , secondAxisFinal.z - this->custom_frame_translationVector.z);



                                        cv::Mat normalizedColum = Util::normalizeVector3D(origin_firstAxisEnd);


                                        this->custom_frame_rotationMatrix.at<float>(0, 1) = normalizedColum.at<float>(0, 0);
                                        this->custom_frame_rotationMatrix.at<float>(1, 1) = normalizedColum.at<float>(1, 0);
                                        this->custom_frame_rotationMatrix.at<float>(2, 1) = normalizedColum.at<float>(2, 0);

                                        
                                        std::cout << "Rotation matrix: " << this->custom_frame_rotationMatrix << std::endl;

                                        //calculate 3rd column
                                        
                                        cv::Vec3f c1 = custom_frame_rotationMatrix.col(0);
                                        cv::Vec3f c2 = custom_frame_rotationMatrix.col(1);

                                        // Compute the third axis (column) as the cross product of c1 and c2
                                        cv::Vec3f c3 = c1.cross(c2);

                                        // Normalize the third axis to make it a unit vector
                                        c3 = c3 / cv::norm(c3);

                                        // Assign the computed third axis back to the third column of the matrix
                                        custom_frame_rotationMatrix.at<float>(0, 2) = c3[0];
                                        custom_frame_rotationMatrix.at<float>(1, 2) = c3[1];
                                        custom_frame_rotationMatrix.at<float>(2, 2) = c3[2];
                                        
                                        std::cout << "c3: " << c3 << std::endl;
                                        std::cout << "Rotation matrix: " << this->custom_frame_rotationMatrix << std::endl;
                                        this->customFrameProcedureCnt++;
                                    }
                                }
                            }
                            
                            
                        }
                    }
                    
                    //ax + ay = z, if 
                }
                
                //this->customFrameProcedureCnt++;
            }
            



            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> contoursHierarchy;
            cv::Mat binaryImage;
            cv::threshold(colorFilter, binaryImage, 254, 1, cv::THRESH_BINARY);
            cv::findContours(binaryImage, contours, contoursHierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
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

            std::vector<std::pair<cv::Point2f, float>> circles;
            float maxRadius = 0;
            int maxRadiusIndex = -1;
            for(int i = 0; i < myHierarchy.size(); i++){
                cv::Point2f newCenter;
                float newRadius;

                int parentId = myHierarchy[i].first;
                std::vector<cv::Point> parentContour = contours[parentId];
                cv::minEnclosingCircle(parentContour, newCenter, newRadius);

                if(newRadius > maxRadius)
                {
                    maxRadius = newRadius;
                    maxRadiusIndex = i;
                }
                circles.push_back(std::make_pair(newCenter, newRadius));
            }    
            
            if(maxRadiusIndex >= 0)
            {
                //printf("Found circles = %d, max_radius = %.2f, x = %.2f, %.2f, %d\n", circles.size(), maxRadius, circles[maxRadiusIndex].first.x, circles[maxRadiusIndex].first.y, maxRadiusIndex);

                std::vector<cv::Point2f> distorted_points;
                std::vector<cv::Point2f> undistorted_points;
                ImVec2 pointScaled;
                pointScaled.x = circles[maxRadiusIndex].first.x;
                pointScaled.y = circles[maxRadiusIndex].first.y;

                this->cam2_2D.x = pointScaled.x;
                this->cam2_2D.y = pointScaled.y;
                this->cam2_2D_updated = true;

                if(this->firstCamShouldIntrinsicalyCorrect)
                {
                    distorted_points.clear();
                    distorted_points.push_back(cv::Point2d(pointScaled.x, pointScaled.y));
                    cv::undistortPoints(distorted_points, undistorted_points, cameraMatrix2, distCoeffs2);
                    cv::Point2d normPoint = undistorted_points.back();
                    pointScaled.x = (normPoint.x * cameraMatrix2.at<double>(0, 0) + cameraMatrix2.at<double>(0, 2)) / (float)this->resolution[0]; // Normalize to image width
                    pointScaled.y = (normPoint.y * cameraMatrix2.at<double>(1, 1) + cameraMatrix2.at<double>(1, 2)) / (float)this->resolution[1]; // Normalize to image height
                
                }
                else{
                    pointScaled.x = pointScaled.x / this->resolution[0];
                    pointScaled.y = pointScaled.y / this->resolution[1];
                }

                if(this->shouldBeColorView)
                {

                    if((pointScaled.x > this->zoom2_A.x) && (pointScaled.x < this->zoom2_B.x)
                        && (pointScaled.y > this->zoom2_A.y) && (pointScaled.y < this->zoom2_B.y)) // draw condition
                    {
                        float x_pixel = ((pointScaled.x - this->zoom2_A.x) / (this->zoom2_B.x - this->zoom2_A.x))*this->resolution[0];
                        float y_pixel = ((pointScaled.y - this->zoom2_A.y) / (this->zoom2_B.y - this->zoom2_A.y))*this->resolution[1];
                        ImVec2 circlePos = ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel);





                        // Your hue, saturation, and value inputs
                        //float hue = (this->hue1[0] + (this->hue1[1] - this->hue1[0]) / 2)*1.41176;
                        //float sat = (this->sat1[0] + (this->sat1[1] - this->sat1[0]) / 2)*1.41176;
                        //float val = (this->val1[0] + (this->val1[1] - this->val1[0]) / 2) * 1.41176;

                        // Convert HSV to RGB using ImGui
                        float r_f = 0.0f, g_f = 0.0f, b_f = 0.0f;
                        // Normalize the input ranges to 0.0 to 1.0
                        float h = (Util::getAvgValue(this->hue2[0], this->hue2[1], 180)) / 180.0f;
                        float s = (this->sat2[0] + (this->sat2[1] - this->sat2[0]) / 2) / 255.0f;
                        float v = (this->val2[0] + (this->val2[1] - this->val2[0]) / 2) / 255.0f;
                        ImGui::ColorConvertHSVtoRGB(h, 1.0f, 1.0f, r_f, g_f, b_f);
                        int r = static_cast<int>(r_f * 255);
                        int g = static_cast<int>(g_f * 255);
                        int b = static_cast<int>(b_f * 255);

                        // Print the result
                        //printf("Red = %d, Green = %d, Blue = %d, h = %.2f, s = %.2f\n", r, g, b);
                        //printf("h = %.2f[%d, %d], s = %.2f[%d], v = %.2f[%d]\n", h, this->hue1[0], this->hue1[1],
                        // s, this->sat1[0] + (this->sat1[1] - this->sat1[0]) / 2,
                        // v, this->val1[0] + (this->val1[1] - this->val1[0]) / 2);


                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        float scalingFactor = 1.0f/(zoom2_B.x-zoom2_A.x);
                        drawList->AddCircle(circlePos, maxRadius*scalingFactor, IM_COL32(r, g, b, 255), 50, 4.0f);
                        float addedTrigonometry = (sqrt(2)/2)*maxRadius*scalingFactor;
                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y - addedTrigonometry), IM_COL32(r, g, b, 120), 2.0f);

                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y - addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y + addedTrigonometry), IM_COL32(r, g, b, 120), 2.0f);


                        addedTrigonometry = (sqrt(2)/2)*(maxRadius > 10 ? 10 : maxRadius);
                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y + addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y - addedTrigonometry), IM_COL32(r, g, b, 255), 4.0f);

                        drawList->AddLine(ImVec2(circlePos.x - addedTrigonometry, circlePos.y - addedTrigonometry),
                        ImVec2(circlePos.x + addedTrigonometry, circlePos.y + addedTrigonometry), IM_COL32(r, g, b, 255), 4.0f);
                    }
                }
                
        
            }


            if(!this->shouldBeColorView && this->calibrate_extrinsic_flag)
            {
                std::vector<cv::Point2f> distorted_points;
                std::vector<cv::Point2f> undistorted_points;
                for(int i = 0; i < this->pairList.size(); i++)
                {
                    ImVec2 pointScaled =  pairList[i].second;
                    if(this->secondCamShouldIntrinsicalyCorrect)
                    {
                        distorted_points.clear();
                        distorted_points.push_back(cv::Point2d(pairList[i].second.x, pairList[i].second.y));
                        cv::undistortPoints(distorted_points, undistorted_points, cameraMatrix2, distCoeffs2);
                        cv::Point2d normPoint = undistorted_points.back();
                        pointScaled.x = (normPoint.x * cameraMatrix2.at<double>(0, 0) + cameraMatrix2.at<double>(0, 2)) / (float)this->resolution[0]; // Normalize to image width
                        pointScaled.y = (normPoint.y * cameraMatrix2.at<double>(1, 1) + cameraMatrix2.at<double>(1, 2)) / (float)this->resolution[1]; // Normalize to image height
                    
                    }
                    else{
                        pointScaled.x = pointScaled.x / this->resolution[0];
                        pointScaled.y = pointScaled.y / this->resolution[1];
                    }

                    if((pointScaled.x > this->zoom2_A.x) && (pointScaled.x < this->zoom2_B.x)
                    && (pointScaled.y > this->zoom2_A.y) && (pointScaled.y < this->zoom2_B.y)) // draw condition
                    {
                        float x_pixel = ((pointScaled.x - this->zoom2_A.x) / (this->zoom2_B.x - this->zoom2_A.x))*this->resolution[0];
                        float y_pixel = ((pointScaled.y - this->zoom2_A.y) / (this->zoom2_B.y - this->zoom2_A.y))*this->resolution[1];
                        ImVec2 circlePos = ImVec2(imagePos.x + x_pixel, imagePos.y + y_pixel);

                        // Draw a red circle
                        if(i == this->closestPairIndex)
                        {
                            drawList->AddCircle(circlePos, 10.0f, IM_COL32(255, 255, 0, 255), 12, 4.0f);
                        }
                        else
                        {
                            drawList->AddCircle(circlePos, 10.0f, IM_COL32(255, 255, 0, 100), 12, 4.0f);
                        }
                    }
                }
            }
            //printf("Dosao %s\n", this->secondCamMsg->camOrigin->frameNickName.c_str());
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        //Draw target frame if enabled..
        ///printf("%d, %d\n", this->showtargetFrameCordSystem, this->worldFrame != nullptr);
        if(this->showtargetFrameCordSystem && (this->worldFrame != nullptr))
        {
            //printf("In\n");
            //there must be path from cam1 to target cord frame
            std::shared_ptr<cv::Mat> transformation_matrix_cam1 = Util::findTransformation(this->worldFrame, cam1);
            
            if(transformation_matrix_cam1 != nullptr)
            {
                cv::Point3f pointOrigin;
                pointOrigin.x = transformation_matrix_cam1->at<double>(0, 3);
                pointOrigin.y = transformation_matrix_cam1->at<double>(1, 3);
                pointOrigin.z = transformation_matrix_cam1->at<double>(2, 3);

                cv::Point2f originLeft = Util::point3D_to_camCordinates(pointOrigin, cam1, cam1);
                cv::Point2f axisXLeft = Util::point3D_to_camCordinates(cv::Point3f(pointOrigin.x + transformation_matrix_cam1->at<double>(0, 0)* this->worldFrameAxisLen, pointOrigin.y + transformation_matrix_cam1->at<double>(0, 1)* this->worldFrameAxisLen, pointOrigin.z + transformation_matrix_cam1->at<double>(0, 2)* this->worldFrameAxisLen), cam1, cam1);
                cv::Point2f axisYLeft = Util::point3D_to_camCordinates(cv::Point3f(pointOrigin.x + transformation_matrix_cam1->at<double>(1, 0)* this->worldFrameAxisLen, pointOrigin.y + transformation_matrix_cam1->at<double>(1, 1)* this->worldFrameAxisLen, pointOrigin.z + transformation_matrix_cam1->at<double>(1, 2)* this->worldFrameAxisLen), cam1, cam1);
                cv::Point2f axisZLeft = Util::point3D_to_camCordinates(cv::Point3f(pointOrigin.x + transformation_matrix_cam1->at<double>(2, 0)* this->worldFrameAxisLen, pointOrigin.y + transformation_matrix_cam1->at<double>(2, 1)* this->worldFrameAxisLen, pointOrigin.z + transformation_matrix_cam1->at<double>(2, 2)* this->worldFrameAxisLen), cam1, cam1);
                
                drawList->AddCircle(ImVec2(imagePosLeft.x + originLeft.x, imagePosLeft.y + originLeft.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);
                drawList->AddLine(ImVec2(imagePosLeft.x + originLeft.x, imagePosLeft.y + originLeft.y), ImVec2(imagePosLeft.x + axisXLeft.x, imagePosLeft.y + axisXLeft.y), IM_COL32(255, 0, 0, 255), 4.0f);
                drawList->AddLine(ImVec2(imagePosLeft.x + originLeft.x, imagePosLeft.y + originLeft.y), ImVec2(imagePosLeft.x + axisYLeft.x, imagePosLeft.y + axisYLeft.y), IM_COL32(0, 255, 0, 255), 4.0f);
                drawList->AddLine(ImVec2(imagePosLeft.x + originLeft.x, imagePosLeft.y + originLeft.y), ImVec2(imagePosLeft.x + axisZLeft.x, imagePosLeft.y + axisZLeft.y), IM_COL32(0, 0, 255, 255), 4.0f);

                //printf("drawn\n");



                cv::Point2f originRight = Util::point3D_to_camCordinates(pointOrigin, cam1, cam2);
                cv::Point2f axisXRight = Util::point3D_to_camCordinates(cv::Point3f(pointOrigin.x + transformation_matrix_cam1->at<double>(0, 0)* this->worldFrameAxisLen, pointOrigin.y + transformation_matrix_cam1->at<double>(0, 1)* this->worldFrameAxisLen, pointOrigin.z + transformation_matrix_cam1->at<double>(0, 2)* this->worldFrameAxisLen), cam1, cam2);
                cv::Point2f axisYRight = Util::point3D_to_camCordinates(cv::Point3f(pointOrigin.x + transformation_matrix_cam1->at<double>(1, 0)* this->worldFrameAxisLen, pointOrigin.y + transformation_matrix_cam1->at<double>(1, 1)* this->worldFrameAxisLen, pointOrigin.z + transformation_matrix_cam1->at<double>(1, 2)* this->worldFrameAxisLen), cam1, cam2);
                cv::Point2f axisZRight = Util::point3D_to_camCordinates(cv::Point3f(pointOrigin.x + transformation_matrix_cam1->at<double>(2, 0)* this->worldFrameAxisLen, pointOrigin.y + transformation_matrix_cam1->at<double>(2, 1)* this->worldFrameAxisLen, pointOrigin.z + transformation_matrix_cam1->at<double>(2, 2)* this->worldFrameAxisLen), cam1, cam2);
                
                drawList->AddCircle(ImVec2(imagePosRight.x + originRight.x, imagePosRight.y + originRight.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);
                drawList->AddLine(ImVec2(imagePosRight.x + originRight.x, imagePosRight.y + originRight.y), ImVec2(imagePosRight.x + axisXRight.x, imagePosRight.y + axisXRight.y), IM_COL32(255, 0, 0, 255), 4.0f);
                drawList->AddLine(ImVec2(imagePosRight.x + originRight.x, imagePosRight.y + originRight.y), ImVec2(imagePosRight.x + axisYRight.x, imagePosRight.y + axisYRight.y), IM_COL32(0, 255, 0, 255), 4.0f);
                drawList->AddLine(ImVec2(imagePosRight.x + originRight.x, imagePosRight.y + originRight.y), ImVec2(imagePosRight.x + axisZRight.x, imagePosRight.y + axisZRight.y), IM_COL32(0, 0, 255, 255), 4.0f);


            }

            //std::shared_ptr<cv::Mat> transformation_matrix_cam2 = Util::findTransformation(this->worldFrame, cam2);

        }


        
        
        cv::Point2f pointLeftOrigin;
        cv::Point2f pointRightOrigin;
        cv::Point2f pointLeftFirstAxis;
        cv::Point2f pointRightFirstAxis;

        if(this->customFrameProcedureCnt == 1) // show origin on first frame
        {
            drawList->AddCircle(ImVec2(imagePosLeft.x + this->custom_frame_origin_left.x, imagePosLeft.y + this->custom_frame_origin_left.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);

        }

        if(this->customFrameProcedureCnt >= 2) // show backangluated origin point to 2d planes
        {

            pointLeftOrigin = Util::point3D_to_camCordinates(this->custom_frame_translationVector, cam1, cam1);
            pointRightOrigin = Util::point3D_to_camCordinates(this->custom_frame_translationVector, cam1, cam2);
            //std::cout << "pointLeft:" << pointLeft << std::endl;
            //std::cout << "pointRight:" << pointRight << std::endl;
            drawList->AddCircle(ImVec2(imagePosLeft.x + pointLeftOrigin.x, imagePosLeft.y + pointLeftOrigin.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);
            drawList->AddCircle(ImVec2(imagePosRight.x + pointRightOrigin.x, imagePosRight.y + pointRightOrigin.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);
        }

        if(this->customFrameProcedureCnt == 3) // show first axis on first image -> like red arrow
        {
            drawList->AddCircle(ImVec2(imagePosLeft.x + this->custom_frame_point1_left.x, imagePosLeft.y + this->custom_frame_point1_left.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);

        }

        if(this->customFrameProcedureCnt >= 4) // show backangluated origin point to 2d planes
        {
            //first column is practicaly first row -> of my first axis... this->custom_frame_rotationMatrix
            
            cv::Point3f axis1End;
            axis1End.x = this->custom_frame_rotationMatrix.at<float>(0, 0) * this->worldFrameAxisLen + this->custom_frame_translationVector.x;
            axis1End.y = this->custom_frame_rotationMatrix.at<float>(1, 0) * this->worldFrameAxisLen + this->custom_frame_translationVector.y;
            axis1End.z = this->custom_frame_rotationMatrix.at<float>(2, 0) * this->worldFrameAxisLen + this->custom_frame_translationVector.z;


            pointLeftFirstAxis = Util::point3D_to_camCordinates(axis1End, cam1, cam1);
            pointRightFirstAxis = Util::point3D_to_camCordinates(axis1End, cam1, cam2);
            //std::cout << "this->custom_frame_rotationMatrix.at<double>(0, 0):" << this->custom_frame_rotationMatrix.at<double>(0, 0) << std::endl;
            //std::cout << "this->custom_frame_rotationMatrix.at<double>(1, 0):" << this->custom_frame_rotationMatrix.at<double>(1, 0) << std::endl;
            //std::cout << "this->custom_frame_rotationMatrix.at<double>(2, 0):" << this->custom_frame_rotationMatrix.at<double>(2, 0) << std::endl;
            //std::cout << "pointLeftOrigin:" << pointLeftOrigin << std::endl;
            //std::cout << "pointRight:" << pointRight << std::endl;
            drawList->AddLine(ImVec2(imagePosLeft.x + pointLeftOrigin.x, imagePosLeft.y + pointLeftOrigin.y), ImVec2(imagePosLeft.x + pointLeftFirstAxis.x, imagePosLeft.y + pointLeftFirstAxis.y), IM_COL32(255, 0, 0, 255), 4.0f);
            drawList->AddLine(ImVec2(imagePosRight.x + pointRightOrigin.x, imagePosRight.y + pointRightOrigin.y), ImVec2(imagePosRight.x + pointRightFirstAxis.x, imagePosRight.y + pointRightFirstAxis.y), IM_COL32(255, 0, 0, 255), 4.0f);
        }

        if(this->customFrameProcedureCnt == 5) // show second axis on left image
        {
            drawList->AddCircle(ImVec2(imagePosLeft.x + this->custom_frame_point2_left.x, imagePosLeft.y + this->custom_frame_point2_left.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);
        }

        if(this->customFrameProcedureCnt >= 6) // show backangluated second and third axis... 
        {
            cv::Point3f axis2End;
            axis2End.x = this->custom_frame_rotationMatrix.at<float>(0, 1) * this->worldFrameAxisLen + this->custom_frame_translationVector.x;
            axis2End.y = this->custom_frame_rotationMatrix.at<float>(1, 1) * this->worldFrameAxisLen + this->custom_frame_translationVector.y;
            axis2End.z = this->custom_frame_rotationMatrix.at<float>(2, 1) * this->worldFrameAxisLen + this->custom_frame_translationVector.z;

            cv::Point3f axis3End;
            axis3End.x = this->custom_frame_rotationMatrix.at<float>(0, 2) * this->worldFrameAxisLen + this->custom_frame_translationVector.x;
            axis3End.y = this->custom_frame_rotationMatrix.at<float>(1, 2) * this->worldFrameAxisLen + this->custom_frame_translationVector.y;
            axis3End.z = this->custom_frame_rotationMatrix.at<float>(2, 2) * this->worldFrameAxisLen + this->custom_frame_translationVector.z;



            cv::Point2f pointLeftSecondAxis = Util::point3D_to_camCordinates(axis2End, cam1, cam1);
            cv::Point2f pointRightSecondAxis = Util::point3D_to_camCordinates(axis2End, cam1, cam2);
            drawList->AddLine(ImVec2(imagePosLeft.x + pointLeftOrigin.x, imagePosLeft.y + pointLeftOrigin.y), ImVec2(imagePosLeft.x + pointLeftSecondAxis.x, imagePosLeft.y + pointLeftSecondAxis.y), IM_COL32(0, 255, 0, 255), 4.0f);
            drawList->AddLine(ImVec2(imagePosRight.x + pointRightOrigin.x, imagePosRight.y + pointRightOrigin.y), ImVec2(imagePosRight.x + pointRightSecondAxis.x, imagePosRight.y + pointRightSecondAxis.y), IM_COL32(0, 255, 0, 255), 4.0f);


            cv::Point2f pointLeftThirdAxis = Util::point3D_to_camCordinates(axis3End, cam1, cam1);
            cv::Point2f pointRightThirdAxis = Util::point3D_to_camCordinates(axis3End, cam1, cam2);
            drawList->AddLine(ImVec2(imagePosLeft.x + pointLeftOrigin.x, imagePosLeft.y + pointLeftOrigin.y), ImVec2(imagePosLeft.x + pointLeftThirdAxis.x, imagePosLeft.y + pointLeftThirdAxis.y), IM_COL32(0, 0, 255, 255), 4.0f);
            drawList->AddLine(ImVec2(imagePosRight.x + pointRightOrigin.x, imagePosRight.y + pointRightOrigin.y), ImVec2(imagePosRight.x + pointRightThirdAxis.x, imagePosRight.y + pointRightThirdAxis.y), IM_COL32(0, 0, 255, 255), 4.0f);
        }

        //Draw all pairs that are visible on both images

        


        if(!this->calibrate_extrinsic_flag)
        {
            if(ImGui::Button("Calibrate extrinsic") || ImGui::IsKeyPressed(ImGuiKey_C))
            {
                this->calibrate_extrinsic_flag = true;
            }
        }
        else
        {
            if(ImGui::Button("Cancel calibration") || ImGui::IsKeyPressed(ImGuiKey_C))
            {
                this->calibrate_extrinsic_flag = false;
            }
        }

        

        if(this->calibrate_extrinsic_flag)
        {
            ImGui::SameLine();
            ImGui::Text(" %d paired points ", this->pairList.size());


            if (this->pairList.size() >= 9)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(70);
                ImGui::DragFloat("prob" , &this->prob, 0.001f, 0.001, 0.999);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(70);
                ImGui::DragFloat("thresh" , &this->threshold, 0.001f, 0.001, 0.999);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(70);
                ImGui::DragInt("iter" , &this->maxIters, 1.0, 1, 5000);
                ImGui::SameLine();
            }
            
            if ((this->pairList.size() >= 9) && ImGui::Button("Calculate_extrinsic")) {


                //std::vector<ImVec2> firstVec = this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName];
                //std::vector<ImVec2> secVec = this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName];
                std::vector<cv::Point2f> imagePoints1;
                std::vector<cv::Point2f> imagePoints2;
                
                for(int i = 0; i < this->pairList.size(); i++){
                    imagePoints1.push_back(cv::Point2f(pairList[i].first.x, pairList[i].first.y));
                    imagePoints2.push_back(cv::Point2f(pairList[i].second.x, pairList[i].second.y));
                }

                printf("Image points size = %d, %d\n", imagePoints1.size(), imagePoints2.size());
                
                std::cout << "cameraMatrix1:" << cameraMatrix1 << std::endl; 
                std::cout << "distCoeffs1:" << distCoeffs1 << std::endl; 
                std::cout << "cameraMatrix2:" << cameraMatrix2 << std::endl; 
                std::cout << "distCoeffs2:" << distCoeffs2 << std::endl; 
                
                cv::Mat R, t;

                printf("Calibration should start\n");
                // Undistort points to normalized image coordinates
                std::vector<cv::Point2f> undistortedPoints1, undistortedPoints2;
                cv::undistortPoints(imagePoints1, undistortedPoints1, cameraMatrix1, distCoeffs1, cv::noArray(), cv::noArray());
                cv::undistortPoints(imagePoints2, undistortedPoints2, cameraMatrix2, distCoeffs2, cv::noArray(), cv::noArray());


                printf("Points undistorted\n");
                for(int j = 0; j < undistortedPoints1.size(); j++){
                    printf("Undistorted point = (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n", undistortedPoints1[j].x, undistortedPoints1[j].y, undistortedPoints2[j].x, undistortedPoints2[j].y
                    , imagePoints1[j].x, imagePoints1[j].y, imagePoints2[j].x, imagePoints2[j].y);
                }
                // Compute the essential matrix using normalized points
                //cv::Mat essentialMatrix = cv::findEssentialMat(undistortedPoints1, undistortedPoints2, cv::Mat::eye(3, 3, CV_64F), cv::RANSAC, 0.999, 1.0);
                //cv::Mat identityMatrix = cv::Mat::eye(3, 3);


                int method = cv::LMEDS;
                
                cv::Mat mask;
                //cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
                cv::Mat identityMatrix = cv::Mat::eye(3, 3, CV_64F);
                cv::Mat essentialMatrix = cv::findEssentialMat(undistortedPoints1, undistortedPoints2, identityMatrix, method, this->prob, this->threshold, this->maxIters, mask);
                std::cout << "Mask (inliers/outliers):" << std::endl;
                for (int i = 0; i < mask.rows; ++i) {
                    std::cout << static_cast<int>(mask.at<uchar>(i)) << " ";
                    
                }

                std::vector<int> outlierIndices;
                for (int i = 0; i < mask.rows; ++i) {
                    if (static_cast<int>(mask.at<uchar>(i)) == 0) {
                        outlierIndices.push_back(i);
                    }
                }

                // Remove outliers in reverse order to avoid shifting issues
                for (int i = outlierIndices.size() - 1; i >= 0; --i) {
                    undistortedPoints1.erase(undistortedPoints1.begin() + outlierIndices[i]);
                    undistortedPoints2.erase(undistortedPoints2.begin() + outlierIndices[i]);
                }
                //cv::Mat essentialMatrix = cv::findEssentialMat(undistortedPoints1, undistortedPoints2, identityMatrix);
                //cv::stereoCalibrate
                //cv::Mat mask;
                //cv::Mat essentialMatrix = cv::findEssentialMat(
                //    imagePoints1, imagePoints2, cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2,
                //    cv::RANSAC, 0.999, 1.0, mask
                //);

                std::cout << "Essential Matrix:\n" << essentialMatrix << std::endl;

                //cv::Mat fundamentalMatrix = cv::findFundamentalMat(undistortedPoints1, undistortedPoints2, cv::FM_8POINT);
                //std::cout << "Fundamental Matrix: " << std::endl << fundamentalMatrix << std::endl;

                
                //cv::findEssentialMat()
                // Recover pose to get rotation and translation
                int inliers = cv::recoverPose(essentialMatrix, undistortedPoints1, undistortedPoints2, R, t);
                std::cout << "Rotation Matrix (R):\n" << R << std::endl;
                std::cout << "Translation Vector (t):\n" << t << std::endl;
                printf("Inliers = %d\n", inliers);
                double beta = std::asin(-R.at<double>(2, 0));
                double alpha = std::atan2(R.at<double>(2, 1), R.at<double>(2, 2));
                double gamma = std::atan2(R.at<double>(1, 0), R.at<double>(0, 0));

                // Convert from radians to degrees
                double alpha_deg = alpha * 180.0 / 3.14159;
                double beta_deg = beta * 180.0 / 3.14159;
                double gamma_deg = gamma * 180.0 / 3.14159;

                std::cout << "Euler angles (in degrees):\n";
                std::cout << "Alpha (rotation around x-axis): " << alpha_deg << std::endl;
                std::cout << "Beta (rotation around y-axis): " << beta_deg << std::endl;
                std::cout << "Gamma (rotation around z-axis): " << gamma_deg << std::endl;
                // Calculate reprojection error
                
                //now i want for every point to say following -> what is points calibration mean, and what is variance, also i want to print all points error in format like:
                


                double avgError = Util::computeReprojectionError(undistortedPoints1, undistortedPoints2, R, t, cameraMatrix1, cameraMatrix2);
                std::cout << "Average Reprojection Error: " << avgError << std::endl;


                std::shared_ptr<FrameRelation> newFrameRelation = std::make_shared<FrameRelation>();
                newFrameRelation->frame_src = cam1;
                newFrameRelation->frame_destination = cam2;
                if(this->currFrameRelation != nullptr)
                {
                    newFrameRelation->distance_between_cams_in_cm = this->currFrameRelation->distance_between_cams_in_cm;
                }
                else
                {
                    newFrameRelation->distance_between_cams_in_cm = 100;
                }

                newFrameRelation->transformation_matrix = cv::Mat::eye(4, 4, R.type()); // Initialize as identity matrix 
                R.copyTo(newFrameRelation->transformation_matrix(cv::Rect(0, 0, 3, 3))); // Copy R into the top-left 3x3 part
                t.copyTo(newFrameRelation->transformation_matrix(cv::Rect(3, 0, 1, 3))); // Copy t into the top-right 3x1 part
                newFrameRelation->transformation_matrix_reprojection_error = avgError;
                std::cout << "Transformation matrix = \n" << newFrameRelation->transformation_matrix << std::endl;
                this->currFrameRelation = newFrameRelation;
                GlobalParams::getInstance().addNewRelation(newFrameRelation);
                //Now i have new rotation/translation parameters and i should add them to global params...
                //FOllow rules -> if cam1-cam2 or cam2-cam1 exist in external calibration -> delete it and create new calibration entry...

                std::cout << "Inverse Transformation Matrix (" <<  newFrameRelation->frame_src->frameNickName << "-" << newFrameRelation->frame_destination->frameNickName << ")" << std::endl;
                std::cout << newFrameRelation->transformation_matrix << std::endl;

                // Compute the inverse transformation matrix (B -> A)
                //cv::Mat T_inverse;
                //cv::invert(newFrameRelation->transformation_matrix, T_inverse);

                // Output the reverse transformation matrix
                std::shared_ptr<FrameRelation> reversed = Util::reverseTransform(newFrameRelation);
                std::cout << "Inverse Transformation Matrix (" <<  reversed->frame_src->frameNickName << "-" << reversed->frame_destination->frameNickName << ")" << std::endl;
                std::cout << reversed->transformation_matrix << std::endl;



                std::shared_ptr<cv::Mat> transformation = Util::findTransformation(cam2, cam1);
                if(transformation == nullptr)
                {
                    printf("Transformation = nullptr\n");
                }
                else
                {
                    printf("Calculated transformation\n");
                    std::cout << *transformation << std::endl; 
                }

                
                //this is for points of cam1
                printf("Cam1 points epipolar line miss in pixels\n");
                for(int i = 0; i < imagePoints1.size(); i++)
                {
                    std::shared_ptr<cv::Point3f> epipolarLineCoef = Util::getEpipolarLineCoefficiens(cam1, imagePoints1[i], cam2);
                    cv::Point2f closesPointOnEpipolarLine = Util::closestPointOnLine(epipolarLineCoef->x, epipolarLineCoef->y, epipolarLineCoef->z,imagePoints2[i]);
                    //now just calculate distance between theese 2 points
                    double dist_in_pix = sqrt((closesPointOnEpipolarLine.x - imagePoints2[i].x)*(closesPointOnEpipolarLine.x - imagePoints2[i].x) + (closesPointOnEpipolarLine.y - imagePoints2[i].y)*(closesPointOnEpipolarLine.y - imagePoints2[i].y));
                    printf("%.2f, %.2f, %.2f\n", imagePoints1[i].x, imagePoints1[i].y, dist_in_pix);
                }

                printf("Cam2 points epipolar line miss in pixels\n");
                for(int i = 0; i < imagePoints2.size(); i++)
                {
                    std::shared_ptr<cv::Point3f> epipolarLineCoef = Util::getEpipolarLineCoefficiens(cam2, imagePoints2[i], cam1);
                    cv::Point2f closesPointOnEpipolarLine = Util::closestPointOnLine(epipolarLineCoef->x, epipolarLineCoef->y, epipolarLineCoef->z,imagePoints1[i]);
                    //now just calculate distance between theese 2 points
                    double dist_in_pix = sqrt((closesPointOnEpipolarLine.x - imagePoints1[i].x)*(closesPointOnEpipolarLine.x - imagePoints1[i].x) + (closesPointOnEpipolarLine.y - imagePoints1[i].y)*(closesPointOnEpipolarLine.y - imagePoints1[i].y));
                    printf("%.2f, %.2f, %.2f\n", imagePoints2[i].x, imagePoints2[i].y, dist_in_pix);
                }
                
                //x1, y1, errorInPixels
                //x2, y2, errorInPixels
                //x3, y3, errorInPixels
                //I want to make like heatMap and interpolate given errors for coresponding points to be able to accurately determine what part of image will be how accurate...
                //for both perspectives...
                //basicaly i need to calculate epipolar line on other view and calculate distance between that line and corresponding point on 2n image.
            }
            

       
        }


        



        if(this->currFrameRelation != nullptr)
        {
            ImGui::SameLine();
            ImGui::Text(std::string(", " + cam1->frameNickName + "<---").c_str());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(75.0f);
            ImGui::DragFloat("##distance_between_cams_in_cm: ", &this->currFrameRelation->distance_between_cams_in_cm, 0.1f, 0.0f, 0.0f, "%.1f cm");
            ImGui::SameLine();
            ImGui::Text(std::string("--->" + cam2->frameNickName).c_str());


            ImGui::Checkbox("showPointLines", &this->showTriangulatedLines);
            ImGui::SameLine();
            ImGui::Checkbox("showPointNumbers", &this->showTriangulatedNumbers);



            if(this->showTriangulatedNumbers)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(120);

                if (ImGui::BeginCombo("##numbers-type", this->cordinateFrameOptions[this->cordinateFrameOption]))
                {
                    // Option 1
                    if (ImGui::Selectable("static", this->cordinateFrameOption == 0))
                    {
                        this->cordinateFrameOption = 0;
                    }
                    // Option 2
                    if (ImGui::Selectable("static-object", this->cordinateFrameOption == 1))
                    {
                        this->cordinateFrameOption = 1;
                    }
                    // Option 3
                    if (ImGui::Selectable("dynamic", this->cordinateFrameOption == 2))
                    {
                        this->cordinateFrameOption = 2;
                    }

                    // Close the combo box when clicked
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(20);
                ImGui::DragInt("font", &this->numbersFont, 0.1f, 3, 20);
            }
        }

        
        


        



        

        
        



        if((this->currFrameRelation != nullptr) && (this->cam1_2D_updated == true) && (this->cam2_2D_updated == true) && !GlobalParams::getInstance().getWorldFrame().empty())
        {
            
            std::shared_ptr<FrameBase> target_frame = GlobalParams::getInstance().getFrame(GlobalParams::getInstance().getWorldFrame());
            
            this->cam1_2D_updated = false;
            this->cam2_2D_updated = false;

            //printf("Dosao je\n");
            //std::cout << "this->cam1_2D" << this->cam1_2D <<std::endl;
            //std::cout << "this->cam2_2D" << this->cam2_2D <<std::endl;
            std::shared_ptr<cv::Point3f> point3D = Util::triangulate(cam1, this->cam1_2D, cam2, this->cam2_2D, target_frame);
            

            //now make axis x to 0, axis y to 0, and axis z to 0...



            if(point3D == nullptr)
            {
                printf("Point3d == nullptr\n");
            }
            else{
                cv::Point2f pointLeft = Util::point3D_to_camCordinates(cv::Point3f(point3D->x, point3D->y, point3D->z), target_frame, cam1);
                cv::Point2f x0Left = Util::point3D_to_camCordinates(cv::Point3f(0, point3D->y, point3D->z), target_frame, cam1);
                cv::Point2f y0Left = Util::point3D_to_camCordinates(cv::Point3f(point3D->x, 0, point3D->z), target_frame, cam1);
                cv::Point2f z0Left = Util::point3D_to_camCordinates(cv::Point3f(point3D->x, point3D->y, 0), target_frame, cam1);


                if(this->showTriangulatedLines)
                {
                    

                    drawList->AddCircle(ImVec2(imagePosLeft.x + pointLeft.x, imagePosLeft.y + pointLeft.y), 5, IM_COL32(255, 0, 0, 255), 15, 3.0f);
                    drawList->AddLine(ImVec2(imagePosLeft.x + pointLeft.x, imagePosLeft.y + pointLeft.y), ImVec2(imagePosLeft.x + x0Left.x, imagePosLeft.y + x0Left.y), IM_COL32(255, 0, 0, 255), 4.0f);
                    drawList->AddLine(ImVec2(imagePosLeft.x + pointLeft.x, imagePosLeft.y + pointLeft.y), ImVec2(imagePosLeft.x + y0Left.x, imagePosLeft.y + y0Left.y), IM_COL32(0, 255, 0, 255), 4.0f);
                    drawList->AddLine(ImVec2(imagePosLeft.x + pointLeft.x, imagePosLeft.y + pointLeft.y), ImVec2(imagePosLeft.x + z0Left.x, imagePosLeft.y + z0Left.y), IM_COL32(0, 0, 255, 255), 4.0f);
                
                }

                if(this->showTriangulatedNumbers)
                {
                    cv::Point2f xTextLeftPos;
                    cv::Point2f yTextLeftPos;
                    cv::Point2f zTextLeftPos;

                    if(this->cordinateFrameOption == 0) // static top
                    {
                        xTextLeftPos.x = 0;
                        xTextLeftPos.y = 0;
                        yTextLeftPos.x = 0;
                        yTextLeftPos.y = 25;
                        zTextLeftPos.x = 0;
                        zTextLeftPos.y = 50;
                    }
                    else if(this->cordinateFrameOption == 1)
                    {
                        xTextLeftPos.x = pointLeft.x + 0;
                        xTextLeftPos.y = pointLeft.y + 0;
                        yTextLeftPos.x = pointLeft.x + 0;
                        yTextLeftPos.y = pointLeft.y + 25;
                        zTextLeftPos.x = pointLeft.x + 0;
                        zTextLeftPos.y = pointLeft.y + 50;
                    }
                    else if(this->cordinateFrameOption == 2)
                    {
                        xTextLeftPos.x = (pointLeft.x + x0Left.x)/2;
                        xTextLeftPos.y = (pointLeft.y + x0Left.y)/2;
                        yTextLeftPos.x = (pointLeft.x + y0Left.x)/2;
                        yTextLeftPos.y = (pointLeft.y + y0Left.y)/2;
                        zTextLeftPos.x = (pointLeft.x + z0Left.x)/2;
                        zTextLeftPos.y = (pointLeft.y + z0Left.y)/2;
                    }


                    // Save the current font size to restore it later
                    float originalFontSize = ImGui::GetFont()->FontSize;

                    // Set a larger font size by pushing a style variable to change the scale
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));  // Optional style change (spacing)
                    ImGui::GetFont()->FontSize = this->numbersFont;  // Set the new font size (e.g., 20)

                    // Render text with the new larger font size
                    drawList->AddText(ImVec2(imagePosLeft.x + xTextLeftPos.x, imagePosLeft.y + xTextLeftPos.y), IM_COL32(255, 0, 0, 255),
                                    boost::str(boost::format("%.1fcm") % point3D->x).c_str());


                    drawList->AddText(ImVec2(imagePosLeft.x + yTextLeftPos.x, imagePosLeft.y + yTextLeftPos.y), IM_COL32(0, 255, 0, 255),
                                    boost::str(boost::format("%.1fcm") % point3D->y).c_str());

                    drawList->AddText(ImVec2(imagePosLeft.x + zTextLeftPos.x, imagePosLeft.y + zTextLeftPos.y), IM_COL32(0, 0, 255, 255),
                                    boost::str(boost::format("%.1fcm") % point3D->z).c_str());

                    // Restore the original font size
                    ImGui::GetFont()->FontSize = originalFontSize;  // Restore to original font size
                    ImGui::PopStyleVar();  // Reset the style variable to the previous state

                }
                
                
                //add doted line
                //add arowhead on end...
                
                //ImGui::SameLine();
                //ImGui::Text("(%s) X = %.3f, Y = %.3f, Z = %.3f", target_frame->frameNickName.c_str(), point3D->x, point3D->y, point3D->z);
            }
            //ImGui::Text("X = ..., Y = ..., Z = ....");
        }

        

        //ImGui::Text("%s-%s", this->externalCalibrationPair.first, this->externalCalibrationPair.second);
        ImGui::EndPopup();
    }
    this->mutex.unlock();
    return;
}



void NodeManualExtrinsic::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);

    if((connector->connectorMessageType == Enums::MessageType::PICTURE) && !this->isShiftPresed){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(message);
        
        if(msg->camOrigin->frameNickName == this->externalCalibrationPair.first)
        {
            //this->firstCamMsg = msg;

            //this->firstCamMsg = std::make_pair(msg->data->first.clone(),  msg->data->second.clone());
            this->mutex.lock();

            if(!(this->historyOfWhatCam)) // means that i should 
            {
                this->camHistory.push_back(std::make_pair(std::make_pair(msg->data->first.clone(), msg->data->second.clone()), msg->getBaseTimestamp()));
                if(this->camHistory.size() >= 100)
                {
                    this->camHistory.erase(this->camHistory.begin());
                }
            }
            else // should try to match within some treshhold. And erase all samples behind...
            {
                int bestMatchIndex = -1;
                int bestMatchDiff = 10000000;

                for(int i = 0; i < this->camHistory.size(); i++)
                {
                    int currDiff = abs(this->camHistory[i].second - msg->getBaseTimestamp());
                    
                    if(currDiff < bestMatchDiff)
                    {
                        bestMatchDiff = currDiff;
                        bestMatchIndex = i;
                    }
                }

                //printf("Best match diff, cam1 = %d\n", bestMatchDiff);

                //errasing all prahistoric elements...

                if((bestMatchIndex == (this->camHistory.size() - 1)) && bestMatchDiff > this->matchIntervalInUs)
                {
                    this->historyOfWhatCam = !this->historyOfWhatCam;
                }

                if(bestMatchDiff < this->matchIntervalInUs)
                {
                    this->firstLastMsg.first = msg->data->first.clone();
                    this->firstLastMsg.second = msg->data->second.clone();

                    this->secondLastMsg.first = this->camHistory[bestMatchIndex].first.first;
                    this->secondLastMsg.second = this->camHistory[bestMatchIndex].first.second;
                }

                if(bestMatchIndex != -1)
                {
                    this->camHistory.erase(this->camHistory.begin(), this->camHistory.begin() + bestMatchIndex);
                }
                
            }



            
            this->mutex.unlock();
        }
        else if(msg->camOrigin->frameNickName == this->externalCalibrationPair.second)
        {
            this->mutex.lock();

            if(this->historyOfWhatCam) // means that i should 
            {
                this->camHistory.push_back(std::make_pair(std::make_pair(msg->data->first.clone(), msg->data->second.clone()), msg->getBaseTimestamp()));
                if(this->camHistory.size() >= 100)
                {
                    this->camHistory.erase(this->camHistory.begin());
                }
            }
            else // should try to match within some treshhold. And erase all samples behind...
            {
                int bestMatchIndex = -1;
                int bestMatchDiff = 10000000;

                for(int i = 0; i < this->camHistory.size(); i++)
                {
                    int currDiff = abs(this->camHistory[i].second - msg->getBaseTimestamp());
                    
                    if(currDiff < bestMatchDiff)
                    {
                        bestMatchDiff = currDiff;
                        bestMatchIndex = i;
                    }
                }

                //printf("Best match diff cam2 = %d\n", bestMatchDiff);

                //errasing all prahistoric elements..., ako se sve briše -> mijenjaj kameru...

                //ako razlika ne zadovoljava -> tada promijeniti referentnu kameru
                if((bestMatchIndex == (this->camHistory.size() - 1)) && bestMatchDiff > this->matchIntervalInUs)
                {
                    this->historyOfWhatCam = !this->historyOfWhatCam;
                }

                if(bestMatchDiff < this->matchIntervalInUs)
                {
                    this->firstLastMsg.first = this->camHistory[bestMatchIndex].first.first;
                    this->firstLastMsg.second = this->camHistory[bestMatchIndex].first.second;

                    this->secondLastMsg.first = msg->data->first.clone();
                    this->secondLastMsg.second = msg->data->second.clone();
                }

                if(bestMatchIndex != -1)
                {
                    this->camHistory.erase(this->camHistory.begin(), this->camHistory.begin() + bestMatchIndex);
                }

                
                
            }


            
            this->mutex.unlock();
        }   

        /*
        if(this->captureFrames)
        {
            if(this->firstCamMsg == nullptr)
            {
                this->firstCamMsg = msg;
            }
            
            else if(this->firstCamMsg->camOrigin->frameNickName != msg->camOrigin->frameNickName)
            {
                this->secondCamMsg = msg;
                this->captureFrames = false;
            }
        }
        */
        //Do nothing, if shift pressed -> freeze!
    }
    
    
    
}


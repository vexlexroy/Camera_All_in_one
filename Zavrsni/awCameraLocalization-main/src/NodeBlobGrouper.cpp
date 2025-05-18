#include "NodeBlobGrouper.hpp"
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
#include <implot.h>
#include <opencv2/core/matx.hpp>
#include <algorithm>
#include <imnodes.h>
#include <boost/circular_buffer.hpp>
#include <unordered_map>
#include <cmath>
#include <cstdlib>

NodeBlobGrouper::NodeBlobGrouper(int uniqueId) : NodeBase(uniqueId){
    this->nodeType = Enums::NodeType::NODEBLOBGROUPER;
}

std::shared_ptr<NodeBase> NodeBlobGrouper::createNewClassInstance(int uniqueId){
    return std::make_shared<NodeBlobGrouper>(uniqueId);
}



std::vector<Enums::MessageType> NodeBlobGrouper::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::BLOB);

    return inMessageTypes;
}



std::vector<Enums::MessageType> NodeBlobGrouper::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::BLOBGROUP);
    
    return outMessageTypes;
}


std::string NodeBlobGrouper::getDescription(){
    return "Ovo je blob grouper node";
}

std::string NodeBlobGrouper::getName(){
    return "Blob-grouper";
}

Enums::NodeType NodeBlobGrouper::getType(){
    return Enums::NodeType::NODEBLOBGROUPER;
}





void NodeBlobGrouper::drawNodeParams(){
    int blobGroupsTimeDiffMs = 5;
    int blobGroupsCnt = 2;
    ImGui::PushItemWidth(50);

    ImGui::Text("PAIRING-RULES: ");
    
    ImGui::DragInt("timeDiff", &this->blobGroupsTimeDiffMs, 1.0, 10, 2000);
    
    ImGui::DragInt("viewHistory", &this->viewLastMs, 1.0, 10, 2000);
    

    ImGui::Text("CALIBRATION: ");
    ImGui::SameLine();
    ImGui::Checkbox("##calibrationActive", &this->calibrationActive);
    if(this->calibrationActive){
        ImGui::SameLine();
        ImGui::ColorEdit4("##CalibrationColor", (float*)&this->calibrationColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine();
        ImGui::DragInt("##minDist", &this->calibrationMinDistance, 1.0f, 10, 100, "pair%dpix");
        ImGui::SameLine();
        ImGui::DragInt("##minPoint", &this->calibrationPointsNum, 1.0f, 10, 500, "cal%dpts");
    }
    


    ImGui::Text("ESTIMATION: ");
    ImGui::SameLine();
    ImGui::Checkbox("##estimationActive", &this->estimationActive);
    if(this->estimationActive){
        ImGui::SameLine();
        ImGui::ColorEdit4("##estimationColor", (float*)&this->estimationColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
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
                        printf("Clicked\n");
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


                //Check if un unordered map exists with cam1cam2, or cam2cam1, eather way return size of bigger entry
                int numOfCollectedPairs = -1;
                if(extrinsicCalibrationPairs.count(openedCamFrames[row]->frameNickName + openedCamFrames[column - 1]->frameNickName) != 0 ){
                    numOfCollectedPairs = extrinsicCalibrationPairs[openedCamFrames[row]->frameNickName + openedCamFrames[column - 1]->frameNickName][openedCamFrames[row]->frameNickName].size();
                }
                else if(extrinsicCalibrationPairs.count(openedCamFrames[column - 1]->frameNickName + openedCamFrames[row]->frameNickName) != 0){
                    numOfCollectedPairs = extrinsicCalibrationPairs[openedCamFrames[column - 1]->frameNickName + openedCamFrames[row]->frameNickName][openedCamFrames[column - 1]->frameNickName].size();
                }
                
                std::string finalValue;

                if(numOfCollectedPairs == -1){
                    finalValue = "-";
                }
                else if(numOfCollectedPairs-1 >= this->calibrationPointsNum){
                    finalValue = "...";
                }
                else{
                    int perc = ((float)(numOfCollectedPairs*100))/this->calibrationPointsNum;
                    finalValue = std::to_string(perc) + "%";
                }

                

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

void NodeBlobGrouper::drawNodeWork(){
    std::unique_lock<std::mutex> lock(this->mutex);

    ImGui::Text("3D point = %.2f, %.2f, %.2f", this->x_point, this->y_point, this->z_point);
    
    //COLORING RULES
    // camMsg -> color of that camera
    // link - gray (unused), yelow (used for calibration), green (used for estimation)

    //If selectAll -> draw everything that is not selected but with much lighter color
    //For every selection draw coresponding line in stronger color
    //Draw all selected lines, link stronger gray

    //First draw selected stuff...
    //msg - selectedStuff, should i Draw and how to draw!

    //("SIZE = %d\n", this->selectedPair.size());

    //Create and unselected list as well if, where each entry will be one way data travel

    std::unordered_map<std::string, int> mapping;
    int yMappedValue = 1;
    std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
    std::vector<std::shared_ptr<FrameCam>> openedCamFrames;
    for(int i = 0; i < camFrames.size(); i++){
        if(camFrames[i]->isConnected){
            openedCamFrames.push_back(camFrames[i]);
            mapping[camFrames[i]->frameNickName] = yMappedValue++;
        }   
    }
    //for(auto it = mapping.begin(); it != mapping.end(); it++){
    //    printf("Mapping[%s] = %d\n", it->first.c_str(), it->second);
    //}

    //Print all selected
    //for(int i = 0; i < this->selectedPair.size(); i++){
    //    printf("%s, %s\n", this->selectedPair[i].first.c_str(), this->selectedPair[i].second.c_str());
    //}
    //printf("\n");//WORKING :)

    //SelectedPair + hoveredPair -> allSelectedPair
    std::vector<std::pair<std::string, std::string>> allSelectedPair(this->selectedPair);
    

    bool hoveredDoesIsNotSelectAllAndIsNotSelected = !std::any_of(this->selectedPair.begin(), this->selectedPair.end(), [hoveredPair = this->hoveredPair](std::pair<std::string, std::string> el){
        return el.first == hoveredPair.first && el.second == hoveredPair.second;
    }) && this->hoveredPair.first != "*" && this->hoveredPair.first != "";
    if(hoveredDoesIsNotSelectAllAndIsNotSelected){
        allSelectedPair.push_back(this->hoveredPair);
    }

    //for(int i = 0; i < allSelectedPair.size(); i++){
    //    printf("%s, %s\n", allSelectedPair[i].first.c_str(), allSelectedPair[i].second.c_str());
    //}
    //printf("\n"); // all good :)



    //Create all unselected nodes...
    
    std::vector<std::pair<std::string, std::string>> allUnselectedPair;
    if(this->selectAll || this->hoveredPair.first == "*"){
        for(int i = 0; i < openedCamFrames.size(); i++){
            for(int j = i; j < openedCamFrames.size(); j++){
                bool exists = std::any_of(allSelectedPair.begin(), allSelectedPair.end(), [str1 = openedCamFrames[j]->frameNickName, str2 = openedCamFrames[i]->frameNickName](std::pair<std::string, std::string> el){
                    return el.first == str1 && el.second == str2;
                });

                if(!exists){
                    allUnselectedPair.push_back(std::make_pair(openedCamFrames[j]->frameNickName, openedCamFrames[i]->frameNickName));
                }
            }
        }
    }

    //for(int i = 0; i < allUnselectedPair.size(); i++){
    //    printf("%s, %s\n", allUnselectedPair[i].first.c_str(), allUnselectedPair[i].second.c_str());
    //}
    //printf("\n"); // all good :)
    

    //Now for each entry draw what is needed

    
    //1. create list of all nodes, that it has its y value, no matter the selection. mapping<string, int>
    //Upscendingly 


    //first to last..., i must first draw all nodes, and then i must draw all connections as well

    // Example window with plot
    if (ImPlot::BeginPlot("Scatter and Line Plot", ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor))) {

        
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(-this->viewLastMs- 200, 200, 0, openedCamFrames.size()+1);
        int pointsPtr = 0;
        long long int currTimestamp = Util::timeSinceEpochMicroseconds();
        int mappingIncrement = 1;


        for(int i = 0; i < allSelectedPair.size(); i++){
            for(auto it = this->data.begin(); it != this->data.end(); it++){
            
                if(allSelectedPair[i].first == it->first){ // means it should be considered

                    for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                        std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
                        entry = *rit;

                        if(allSelectedPair[i].second == allSelectedPair[i].first){ // draw point...
                            float msBeforeCurrTimestamp =  (entry->second->getBaseTimestamp() - currTimestamp)/1000;
                            //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                            if(std::abs(msBeforeCurrTimestamp) > this->viewLastMs){
                                continue;
                            }

                            this->points[pointsPtr++] = msBeforeCurrTimestamp;
                            this->points[pointsPtr++] = mapping[it->first];
                            int firstPointx = pointsPtr - 2;
                            int firstPointy = pointsPtr - 1;


                            ImVec4 pointColor = this->camColor[it->first];
                            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 3.0f, pointColor);
                            ImPlot::PushStyleColor(ImPlotCol_Line, pointColor);
                            ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, pointColor);
                            ImPlot::PushStyleColor(ImPlotCol_MarkerFill, pointColor);

                            ImPlot::PlotScatter(it->first.c_str(), this->points + firstPointx, this->points + firstPointy, 1);
                            
                            ImPlot::PopStyleColor(3);
                        }
                        else{ // draw line... (only line that corresponds to ), do not for now
                            float msBeforeCurrTimestamp =  (entry->second->getBaseTimestamp() - currTimestamp)/1000;
                            //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                            if(std::abs(msBeforeCurrTimestamp) > this->viewLastMs){
                                continue;
                            }

                            for(int i = 0; i < entry->first.size(); i++){
                                
                                if(entry->first[i].first != allSelectedPair[i].second){
                                    continue;
                                }

                                float msBeforeCurrTimestamp2 =  (entry->first[i].second.first - currTimestamp)/1000;
                                //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                                if(std::abs(msBeforeCurrTimestamp2) > this->viewLastMs){
                                    continue;
                                }

                                
                                //Draw this line...
                                this->points[pointsPtr++] = msBeforeCurrTimestamp;
                                this->points[pointsPtr++] = msBeforeCurrTimestamp2;
                                this->points[pointsPtr++] = mapping[it->first];
                                this->points[pointsPtr++] = mapping[entry->first[i].first];


                                int xPointsPtr = pointsPtr - 4;
                                int yPointsPtr = pointsPtr - 2;

                                ImVec4 color;
                                if(entry->first[i].second.second == Enums::PairStatus::UNUSED){
                                    color = this->defaultColor;
                                }
                                else if(entry->first[i].second.second == Enums::PairStatus::CALIBRATION){
                                    color = this->calibrationColor;
                                }
                                else if(entry->first[i].second.second == Enums::PairStatus::ESTIMATION){
                                    color = this->estimationColor;
                                }

                                ImPlot::PushStyleColor(ImPlotCol_Line, color);
                                ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, color);
                                ImPlot::PushStyleColor(ImPlotCol_MarkerFill, color);
                                ImPlot::PlotLine((it->first + "-" + entry->first[i].first).c_str() , this->points + xPointsPtr, this->points + yPointsPtr, 2);
                                ImPlot::PopStyleColor(3);
                            }
                        }
                    }
                }

                
            }
            
        }
    
        //printf("this->hoveredPair.first= %s\n", this->hoveredPair.first.c_str());
        if(this->selectAll || this->hoveredPair.first == "*"){
            for(int i = 0; i < allUnselectedPair.size(); i++){
                for(auto it = this->data.begin(); it != this->data.end(); it++){
                
                    if(allUnselectedPair[i].first == it->first){ // means it should be considered

                        for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                            std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
                            entry = *rit;

                            if(allUnselectedPair[i].second == allUnselectedPair[i].first){ // draw point...
                                float msBeforeCurrTimestamp =  (entry->second->getBaseTimestamp() - currTimestamp)/1000;
                                //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                                if(std::abs(msBeforeCurrTimestamp) > this->viewLastMs){
                                    continue;
                                }

                                this->points[pointsPtr++] = msBeforeCurrTimestamp;
                                this->points[pointsPtr++] = mapping[it->first];
                                int firstPointx = pointsPtr - 2;
                                int firstPointy = pointsPtr - 1;


                                ImVec4 pointColor = this->camColor[it->first];
                                pointColor.w = 0.3f;
                                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 3.0f, pointColor);
                                ImPlot::PushStyleColor(ImPlotCol_Line, pointColor);
                                ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, pointColor);
                                ImPlot::PushStyleColor(ImPlotCol_MarkerFill, pointColor);

                                ImPlot::PlotScatter(it->first.c_str(), this->points + firstPointx, this->points + firstPointy, 1);
                                
                                ImPlot::PopStyleColor(3);
                            }
                            else{ // draw line... (only line that corresponds to ), do not for now
                                float msBeforeCurrTimestamp =  (entry->second->getBaseTimestamp() - currTimestamp)/1000;
                                //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                                if(std::abs(msBeforeCurrTimestamp) > this->viewLastMs){
                                    continue;
                                }

                                for(int i = 0; i < entry->first.size(); i++){
                                    
                                    if(entry->first[i].first != allUnselectedPair[i].second){
                                        continue;
                                    }

                                    float msBeforeCurrTimestamp2 =  (entry->first[i].second.first - currTimestamp)/1000;
                                    //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                                    if(std::abs(msBeforeCurrTimestamp2) > this->viewLastMs){
                                        continue;
                                    }

                                    
                                    //Draw this line...
                                    this->points[pointsPtr++] = msBeforeCurrTimestamp;
                                    this->points[pointsPtr++] = msBeforeCurrTimestamp2;
                                    this->points[pointsPtr++] = mapping[it->first];
                                    this->points[pointsPtr++] = mapping[entry->first[i].first];


                                    int xPointsPtr = pointsPtr - 4;
                                    int yPointsPtr = pointsPtr - 2;

                                    ImVec4 color;
                                    if(entry->first[i].second.second == Enums::PairStatus::UNUSED){
                                        color = this->defaultColor;
                                    }
                                    else if(entry->first[i].second.second == Enums::PairStatus::CALIBRATION){
                                        color = this->calibrationColor;
                                    }
                                    else if(entry->first[i].second.second == Enums::PairStatus::ESTIMATION){
                                        color = this->estimationColor;
                                    }
                                    color.w = 0.3f;
                                    ImPlot::PushStyleColor(ImPlotCol_Line, color);
                                    ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, color);
                                    ImPlot::PushStyleColor(ImPlotCol_MarkerFill, color);
                                    ImPlot::PlotLine((it->first + "-" + entry->first[i].first).c_str() , this->points + xPointsPtr, this->points + yPointsPtr, 2);
                                    ImPlot::PopStyleColor(3);
                                }
                            }
                        }
                    }

                    
                }
                
            }
        }
        
        /*
        for(auto it = this->data.begin(); it != this->data.end(); it++){
            mapping[it->first] = mappingIncrement++;
            for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
                entry = *rit;
                

                //DRAWING SELECTED STUFF...




                float msBeforeCurrTimestamp =  (entry->second->getBaseTimestamp() - currTimestamp)/1000;
                //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                if(std::abs(msBeforeCurrTimestamp) > this->viewLastMs){
                    continue;
                }

                this->points[pointsPtr++] = msBeforeCurrTimestamp;
                this->points[pointsPtr++] = mapping[it->first];
                int firstPointx = pointsPtr - 2;
                int firstPointy = pointsPtr - 1;
                ImPlot::PlotScatter(entry->second->camOrigin->frameNickName.c_str(), this->points + firstPointx, this->points + firstPointy, 1);
            
                
            }
        }

        for(auto it = this->data.begin(); it != this->data.end(); it++){
            for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
                entry = *rit;
                
                float msBeforeCurrTimestamp =  (entry->second->getBaseTimestamp() - currTimestamp)/1000;
                //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                if(std::abs(msBeforeCurrTimestamp) > this->viewLastMs){
                    continue;
                }



                for(int i = 0; i < entry->first.size(); i++){
                    
                    float msBeforeCurrTimestamp2 =  (entry->first[i].second.first - currTimestamp)/1000;
                    //printf("%f, startLagDuration = %lld\n", msBeforeCurrTimestamp, entry->second->startLagDuration);
                    if(std::abs(msBeforeCurrTimestamp2) > this->viewLastMs){
                        continue;
                    }

                    
                    if(mapping[it->first] < mapping[entry->first[i].first]){ // samo u jednom pravcu crtaj dužine, da se ne crtaju dvije...
                        //Draw this line...
                        this->points[pointsPtr++] = msBeforeCurrTimestamp;
                        this->points[pointsPtr++] = msBeforeCurrTimestamp2;
                        this->points[pointsPtr++] = mapping[it->first];
                        this->points[pointsPtr++] = mapping[entry->first[i].first];


                        int xPointsPtr = pointsPtr - 4;
                        int yPointsPtr = pointsPtr - 2;

                        ImPlot::PlotLine((it->first + "-" + entry->first[i].first).c_str() , this->points + xPointsPtr, this->points + yPointsPtr, 2);
                    }
                }
            }
        }
        */
   
        /*
        // Plot scatter plot for Data 1
        ImPlot::PlotScatter("Data 1", xs1, ys1, 1);
        ImPlot::PlotScatter("Data 1", xs1+1, ys1+1, 1);

        // Plot line for Data 1
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2);
        ImPlot::SetNextLineStyle(ImVec4(1, 0, 0, 1), 2.0f); // Red color (1, 0, 0, 1) and line weight 2.0
        ImPlot::PlotLine("Data 1", xs1, ys1, 50);
        ImPlot::PopStyleVar();

        // Plot scatter plot for Data 2
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 6, ImPlot::GetColormapColor(1), IMPLOT_AUTO, ImPlot::GetColormapColor(1));
        ImPlot::PlotScatter("Data 2", xs2, ys2, 50);
        ImPlot::PopStyleVar();
        */
        // End plotting
        ImPlot::EndPlot();
        //printf("pointsPtr = %d\n", pointsPtr);
    }
    
    //Here draw all entries that started calibration journey...


    
    if(this->calibrationActive){
        for(int i = 0; i < allSelectedPair.size(); i++){
            //ImGui::Text("JAo");
            if(this->extrinsicCalibrationPairs.count(allSelectedPair[i].second + allSelectedPair[i].first) == 0){
                continue;
            }
            std::vector<ImVec2> firstPoints = this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].second];
            std::vector<ImVec2> secondPoints = this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].first];


            //Taj pair se počeo kalibrirati i odabran je...
            ImGui::Text((allSelectedPair[i].second + allSelectedPair[i].first + std::to_string(this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].second].size())).c_str());
            // Start a new plot
            if (ImPlot::BeginPlot((allSelectedPair[i].second + " - " +allSelectedPair[i].first).c_str(), nullptr,  nullptr, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor,240*GlobalParams::getInstance().getZoom().scaleFactor), ImPlotFlags_NoLegend)) {
                // Set marker style (no lines, only markers)
                ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations);
                ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_NoDecorations);

                ImPlot::SetupAxisLimits(ImAxis_X1, -this->resolution[0], this->resolution[0], ImPlotCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, this->resolution[1], ImPlotCond_Always);

                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1.0f, ImVec4(0, 0, 0, 0), 1.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // red circles
                
                float xLine[2] = {0, 0};
                float yLine[2] = {0, this->resolution[1]};
                ImPlot::SetNextMarkerStyle(ImPlotMarker_None);
                ImPlot::PlotLine("SeparationLine", xLine, yLine, 2);

                ImPlotPoint point;
                int minDist = 5000;
                int indexThatIsNearest = -1;
                if(ImPlot::IsPlotHovered()){
                    point = ImPlot::GetPlotMousePos();
                    point.y = this->resolution[1]-point.y;

                    for(int j = 1; j < firstPoints.size(); j++){
                        double dist1 = sqrt(pow(point.x+this->resolution[0]-firstPoints[j].x, 2) + pow(point.y-firstPoints[j].y, 2)); // add this->resolution[0] in x
                        double dist2 = sqrt(pow(point.x-secondPoints[j].x, 2) + pow(point.y-secondPoints[j].y, 2));

                        if(dist1 < minDist){
                            minDist = dist1;
                            indexThatIsNearest = j;
                        }
                        if(dist2 < minDist){
                            minDist = dist2;
                            indexThatIsNearest = j;
                        }
                    }

                    //printf("Mouse = %.2f, %.2f\n", point.x, point.y);
                }
                if(indexThatIsNearest != -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){ // remove pair with that index
                    this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].second].erase(
                        this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].second].begin() + indexThatIsNearest
                    );

                    this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].first].erase(
                        this->extrinsicCalibrationPairs[allSelectedPair[i].second + allSelectedPair[i].first][allSelectedPair[i].first].begin() + indexThatIsNearest
                    );
                }
                
                for(int j = 0; j < firstPoints.size(); j++){
                    ImVec2 firstPoint = ImVec2(firstPoints[j].x - this->resolution[0], this->resolution[1]-firstPoints[j].y);
                    ImVec2 secondPoint = ImVec2(secondPoints[j].x, this->resolution[1]-secondPoints[j].y);

                    if(j == 0){ // Write curr data with x
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Cross, 10.0f);
                        ImPlot::PlotScatter("CurrentL", &firstPoint.x, &firstPoint.y, 1);

                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Cross, 10.0f);
                        ImPlot::PlotScatter("CurrentR", &secondPoint.x, &secondPoint.y, 1);
                    }
                    else if(j == indexThatIsNearest){ // write magnified both entries, and add point num

                        ImFont* font = ImGui::GetFont();
                        font->Scale = 1.5;
                        ImGui::PushFont(font);

                        //ImPlot::PushStyleVar(ImPlotMarker_Circle)
                        //ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f);
                        //ImPlot::PlotScatter("CurrentL", &firstPoint.x, &firstPoint.y, 1);
                        ImPlot::PlotText(std::to_string(j).c_str(), firstPoint.x, firstPoint.y-17);

                        //ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f);
                        //ImPlot::PlotScatter("CurrentR", &secondPoint.x, &secondPoint.y, 1);
                        ImPlot::PlotText(std::to_string(j).c_str(), secondPoint.x, secondPoint.y-17);

                        //DrawLineBetween them...
                        float xLine[2] = {firstPoint.x, secondPoint.x};
                        float yLine[2] = {firstPoint.y, secondPoint.y};
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_None);
                        ImPlot::PlotLine("ConnLine", xLine, yLine, 2);

                        //Restoring font size..
                        ImGui::PopFont();   
                        font->Scale = 1;
                        ImGui::PushFont(font);
                        ImGui::PopFont();

                        
                    }
                    else{ // write standard
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f);
                        //ImPlot::PlotScatter("CurrentL", &firstPoint.x, &firstPoint.y, 1);
                        ImPlot::PlotText(std::to_string(j).c_str(), firstPoint.x, firstPoint.y);

                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f);
                        //ImPlot::PlotScatter("CurrentR", &secondPoint.x, &secondPoint.y, 1);
                        ImPlot::PlotText(std::to_string(j).c_str(), secondPoint.x, secondPoint.y);
                    }

                }

                
                //float x[] = {-500, -300, -200, 200, 450};
                //float y[] = {20, 75, 120, 45, 450};
                
                // Plot the scatter data
                //ImPlot::PlotScatter("Scatter", x, y, 5);
                //1. odredi koja točka je 
                
                
                
                // End the plot
                ImPlot::EndPlot();
            }

            
            
        }
    }

    
    

    
    
    return;
}


void NodeBlobGrouper::recieve(std::shared_ptr<MessageBase> message, int connectorId){
    std::shared_ptr<ConnectorBase> connector = this->getConnector(connectorId);
    
    if(message->messageType == Enums::MessageType::BLOB){
        std::unique_lock<std::mutex> lock(this->mutex);
        std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>(message);
        //printf("%d\n", msg->camOrigin->streamLagInMilisecond);
        
        //CREATING NEW CAM ENTRY IF DOES NOT EXIST
        if(this->data.find(msg->camOrigin->frameNickName) == this->data.end()){
            boost::circular_buffer<std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>> cb(100);
            this->data[msg->camOrigin->frameNickName] = cb;
            //printf("Added first item..., %s\n", msg->camOrigin->frameNickName.c_str());
        }

        //ADDING NEW DATA TO DATA
        std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>> pairedFramesStrings;
        std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>> 
        pair = std::make_shared<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
        (pairedFramesStrings, msg);
        //printf("%d\n", pair->second->camOrigin->streamLagInMilisecond);
        this->data[msg->camOrigin->frameNickName].push_back(pair);
        


        //printf("Size = %d\n", this->data[msg->camOrigin->frameNickName].size());
        

        //Should see all last entries and if 

        //Now try to pair arrived blob to any other frame
        //Every frame should try to connect to closest frame in time
        std::vector<std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>> matchedResults;
        for(auto it = this->data.begin(); it != this->data.end(); it++){
            //it->first -> key
            //Samo trebam iterirati kroz sve streamove koji nisu pristigli i trebam vidjeti jel razlika timestampova 
            
            if(it->first != msg->camOrigin->frameNickName){
                std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
                bestMatch;
                long long int bestMatchTimestampDiff = 100000;
                //printf("SHould decline\n");
                
                for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                    //if(this->realTimeMode && (rit != it->second.rbegin())){ //Realtime considers only firs element...
                    //    break;
                    //}

                    std::shared_ptr<std::pair<std::vector<std::pair<std::string, std::pair<long long int, Enums::PairStatus>>>, std::shared_ptr<Message<std::shared_ptr<std::vector<Structs::BlobDescription>>>>>>
                    entry = *rit;

                    //If already connected between each other than just continue...
                    bool isAlreadyGrouped = false;
                    for(int i = 0; i < entry->first.size(); i++){
                        if(entry->first[i].first == msg->camOrigin->frameNickName){
                            isAlreadyGrouped = true;
                            break;
                        }
                    }
                    if(isAlreadyGrouped){
                        continue;
                    }

                    //It is not grouped, it has chance to get paired, i just need to find closest one...
                    //Calculate difference between arrived frame and frame from past.
                    
                    long long int timestamp1 = msg->getBaseTimestamp();
                    long long int timestamp2 = entry->second->getBaseTimestamp();
                    long long int matchTimestampDiff = std::abs(timestamp2 - timestamp1) / 1000;
                    
                    //printf("matchTimestampDiff = %d, bestMatchTimestampDiff= %d, this->blobGroupsTimeDiffMs = %d\n"
                    //, matchTimestampDiff, bestMatchTimestampDiff, this->blobGroupsTimeDiffMs);
                    if((matchTimestampDiff < this->blobGroupsTimeDiffMs) 
                    &&  (matchTimestampDiff < bestMatchTimestampDiff)){
                        bestMatchTimestampDiff = matchTimestampDiff;
                        bestMatch = entry;
                    }
                    
                }

                if(bestMatch != nullptr){
                    matchedResults.push_back(bestMatch);
                }
                
            }
        }



        //printf("Matched results size = %d\n", matchedResults.size());
        
        //sortiraj matched results od najkasnijeg do najranijeg... (gledaj avg npr...)
        std::sort(matchedResults.begin(), matchedResults.end(), [](const auto& a, const auto& b) {
            return a->second->getBaseTimestamp() < b->second->getBaseTimestamp();
        });


        std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
        std::vector<std::shared_ptr<FrameCam>> openedCamFrames;
        for(int i = 0; i < camFrames.size(); i++){
            if(camFrames[i]->isConnected){
                openedCamFrames.push_back(camFrames[i]);
            }   
        }


        for(int i = 0; i < matchedResults.size(); i++){
            //printf("%lld\n", matchedResults[i]->second->getBaseTimestamp()); it is ok
            
            //long long int timestampMiddle = (msg->getBaseTimestamp() + matchedResults[i]->second->getBaseTimestamp())/2; // conversion is neglectible
            
            /*
            if(this->realTimeMode){
                if(timestampMiddle < this->timestampRealTimeCnt){
                    continue;
                }
                else{
                    this->timestampRealTimeCnt = timestampMiddle;
                }
            }
            */
            Enums::PairStatus pairStatus = Enums::PairStatus::UNUSED;//Can simulate by changing to other option
            
            //Calibration uses just 1 object on frame, takes biggest...
            if(this->calibrationActive){ // 
                //std::string calibrationEntry = pair
                std::string combinedString;
                //std::pair<>
                for(int j = 0; j < openedCamFrames.size(); j++){
                    if(openedCamFrames[j]->frameNickName == matchedResults[i]->second->camOrigin->frameNickName){
                        combinedString += openedCamFrames[j]->frameNickName;
                    }
                    if(openedCamFrames[j]->frameNickName == pair->second->camOrigin->frameNickName){
                        combinedString += openedCamFrames[j]->frameNickName;
                    }
                }
                
                if(this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].size() == 0){
                    ImVec2 firstMiddle = (*(matchedResults[i]->second->data))[0].middle;
                    this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].push_back(
                        firstMiddle);
                    this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].push_back(
                        firstMiddle);

                    ImVec2 secondMiddle = (*(pair->second->data))[0].middle;
                    this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName].push_back(
                        secondMiddle);
                    this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName].push_back(
                        firstMiddle);
                    pairStatus = Enums::PairStatus::CALIBRATION;
                }
                else if(this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].size() < this->calibrationPointsNum){
                    ImVec2 firstMiddle = (*(matchedResults[i]->second->data))[0].middle;
                    ImVec2 secondMiddle = (*(pair->second->data))[0].middle;
                    

                    int minRecDistance = 10000;

                    for(int j = 1; j < this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].size(); j++){
                        double minDist1 = sqrt(
                        (this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName][j].x - firstMiddle.x)
                        *(this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName][j].x - firstMiddle.x)
                        +
                        (this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName][j].y - firstMiddle.y)
                        *(this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName][j].y - firstMiddle.y)
                        
                        );

                        double minDist2 = sqrt(
                        (this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName][j].x - secondMiddle.x)
                        *(this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName][j].x - secondMiddle.x)
                        +
                        (this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName][j].y - secondMiddle.y)
                        *(this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName][j].y - secondMiddle.y)
                        );


                        if(minDist1 < minRecDistance){
                            minRecDistance = minDist1;
                        }
                        if(minDist2 < minRecDistance){
                            minRecDistance = minDist2;
                        }
                    }
                    
                    if(minRecDistance >= this->calibrationMinDistance){
                        this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName][0] = firstMiddle;
                        this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].push_back(
                            firstMiddle);

                        
                        this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName][0] = secondMiddle;
                        this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName].push_back(
                            secondMiddle);
                        pairStatus = Enums::PairStatus::CALIBRATION;
                    }

                    
                    if(this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].size() == this->calibrationPointsNum){
                        //get intrinsic calib of cam1, and cam2
                        //get all points of cam1 and cam2
                        std::shared_ptr<FrameCam> cam1;
                        std::shared_ptr<FrameCam> cam2;
                        for(int j = 0; j < openedCamFrames.size(); j++){
                            if(openedCamFrames[j]->frameNickName == matchedResults[i]->second->camOrigin->frameNickName){
                                cam1 = openedCamFrames[j];
                            }
                            else if(openedCamFrames[j]->frameNickName == pair->second->camOrigin->frameNickName){
                                cam2 = openedCamFrames[j];
                            }
                        }

                        
                        cv::Mat cameraMatrix1 = cam1->intrinsicParams.intrinsicMatrix; // Fill in with your camera matrix values
                        cv::Mat distCoeffs1 = cam1->intrinsicParams.distortionCoef;   // Fill in with your distortion coefficients
                        cv::Mat cameraMatrix2 = cam2->intrinsicParams.intrinsicMatrix; // Fill in with your camera matrix values
                        cv::Mat distCoeffs2 = cam2->intrinsicParams.distortionCoef;



                        std::vector<ImVec2> firstVec = this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName];
                        std::vector<ImVec2> secVec = this->extrinsicCalibrationPairs[combinedString][pair->second->camOrigin->frameNickName];
                        std::vector<cv::Point2f> imagePoints1;
                        std::vector<cv::Point2f> imagePoints2;
                        
                        for(int j = 1; j < firstVec.size(); j++){
                            imagePoints1.push_back(cv::Point2f(firstVec[j].x, firstVec[j].y));
                            imagePoints2.push_back(cv::Point2f(secVec[j].x, secVec[j].y));
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
                        int method = cv::RANSAC;
                        double prob = 0.9999;
                        double threshold = 0.003;
                        int maxIters = 5000;
                        cv::Mat mask;
                        //cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
                        cv::Mat identityMatrix = cv::Mat::eye(3, 3, CV_64F);
                        cv::Mat essentialMatrix = cv::findEssentialMat(undistortedPoints1, undistortedPoints2, identityMatrix, method, prob, threshold, maxIters, mask);
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
                        this->rotationMatrix = R;
                        this->translationMatrix = t;
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
                        double avgError = Util::computeReprojectionError(undistortedPoints1, undistortedPoints2, R, t, cameraMatrix1, cameraMatrix2);
                        std::cout << "Average Reprojection Error: " << avgError << std::endl;

                        
                    }
                    //printf("Size = %d\n", this->extrinsicCalibrationPairs[combinedString][matchedResults[i]->second->camOrigin->frameNickName].size());
                }
                

                if((!this->rotationMatrix.empty()) && (!this->translationMatrix.empty()))
                {
                    std::shared_ptr<FrameCam> cam1;
                    std::shared_ptr<FrameCam> cam2;

                    // Find the cameras corresponding to the matched results
                    for (int j = 0; j < openedCamFrames.size(); j++) {
                        if (openedCamFrames[j]->frameNickName == matchedResults[i]->second->camOrigin->frameNickName) {
                            cam1 = openedCamFrames[j];
                        } else if (openedCamFrames[j]->frameNickName == pair->second->camOrigin->frameNickName) {
                            cam2 = openedCamFrames[j];
                        }
                    }

                    if (!cam1 || !cam2) {
                        std::cerr << "Error: Unable to find corresponding cameras for the matched results." << std::endl;
                        return;
                    }

                    // Camera parameters
                    cv::Mat cameraMatrix1 = cam1->intrinsicParams.intrinsicMatrix;
                    cv::Mat distCoeffs1 = cam1->intrinsicParams.distortionCoef;
                    cv::Mat cameraMatrix2 = cam2->intrinsicParams.intrinsicMatrix;
                    cv::Mat distCoeffs2 = cam2->intrinsicParams.distortionCoef;

                    //std::cout << "cameraMatrix1:" << cameraMatrix1 << std::endl;
                    //std::cout << "distCoeffs1:" << distCoeffs1 << std::endl;
                    //std::cout << "cameraMatrix2:" << cameraMatrix2 << std::endl;
                    //std::cout << "distCoeffs2:" << distCoeffs2 << std::endl;

                    // Feature points in pixel coordinates
                    ImVec2 firstMiddle = (*(matchedResults[i]->second->data))[0].middle;
                    ImVec2 secondMiddle = (*(pair->second->data))[0].middle;

                    //std::cout << "firstMiddle:" << firstMiddle.x << ", " << firstMiddle.y << std::endl;
                    //std::cout << "secondMiddle:" << secondMiddle.x << ", " << secondMiddle.y << std::endl;

                    // Convert ImVec2 to cv::Point2f
                    std::vector<cv::Point2f> points1 = { cv::Point2f(firstMiddle.x, firstMiddle.y) };
                    std::vector<cv::Point2f> points2 = { cv::Point2f(secondMiddle.x, secondMiddle.y) };

                    // Undistort points
                    std::vector<cv::Point2f> undistortedPoints1, undistortedPoints2;
                    cv::undistortPoints(points1, undistortedPoints1, cameraMatrix1, distCoeffs1);
                    cv::undistortPoints(points2, undistortedPoints2, cameraMatrix2, distCoeffs2);

                    if (undistortedPoints1.empty() || undistortedPoints2.empty()) {
                        std::cerr << "Error: Undistorted points are empty." << std::endl;
                        return;
                    }
                    // Projection matrices (P1 and P2)
                    // Assuming P1 is the identity matrix and P2 is based on the relative pose (R, t) between cameras
                    cv::Mat R, t;
                    // Obtain R and t from the camera extrinsic parameters
                    cv::Mat P1 = cv::Mat::eye(3, 4, CV_64F); // Camera 1 projection matrix (identity)
                    cv::Mat P2(3, 4, CV_64F);               // Camera 2 projection matrix
                    this->rotationMatrix.copyTo(P2(cv::Rect(0, 0, 3, 3)));     // Copy rotation into P2
                    this->translationMatrix.copyTo(P2(cv::Rect(3, 0, 1, 3)));     // Copy translation into P2
                    // Triangulate points
                    cv::Mat points4D;
                    cv::triangulatePoints(P1, P2, undistortedPoints1, undistortedPoints2, points4D);

                    // Convert homogeneous coordinates to Euclidean
                    for (int i = 0; i < points4D.cols; ++i) {
                        cv::Mat point = points4D.col(i);
                        point /= point.at<float>(3); // Normalize homogeneous coordinates

                        this->x_point = point.at<float>(0);
                        this->y_point = point.at<float>(1);
                        this->z_point = point.at<float>(2);

                    }


                }
            }

            
            

            //HERE I CAN CHANGE PAIR STATUS IF THERE IS A CALIBRATION MATCH, OR THERE IS ESTIMATION AVAILABLE
            

            //Ostaju svi oni koje treba upariti i poslati... pair && matchedResults[i] upari...
            pair->first.push_back(std::make_pair(matchedResults[i]->second->camOrigin->frameNickName,  std::make_pair(matchedResults[i]->second->getBaseTimestamp(), pairStatus)));
            matchedResults[i]->first.push_back(std::make_pair(pair->second->camOrigin->frameNickName, std::make_pair(pair->second->getBaseTimestamp(), pairStatus)));
            

        }


        //Za real time pazi da updataš realTimeCnt accoridngly...
        

        //Spoji sve matched results sa trenutnim framom i pošalji dalje rezultate
    }    
}





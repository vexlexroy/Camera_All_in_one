#include "NodeSourceStream.hpp"
#include "Enums.hpp"
#include "ElementFactory.hpp"
#include "Util.hpp"
#include "Message.hpp"
#include "Structs.hpp"
#include "GlobalParams.hpp"

#include <memory>
#include <vector>
#include <iostream>
#include <imgui.h>
#include <cstdlib> // For std::atoi
#include <cctype>  // For std::isdigit
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <boost/algorithm/string.hpp>


NodeSourceStream::NodeSourceStream(int uniqueId) : NodeSource(uniqueId){
    this->nodeType = Enums::NodeType::NODESOURCESTREAM;
}

std::shared_ptr<NodeBase> NodeSourceStream::createNewClassInstance(int uniqueId){
    std::shared_ptr<NodeSourceStream> newNode = std::make_shared<NodeSourceStream>(uniqueId);
    newNode->startThread();
    return newNode;
}
std::vector<Enums::MessageType> NodeSourceStream::getInMessageTypes(){
    std::vector<Enums::MessageType> inMessageTypes;
    inMessageTypes.push_back(Enums::MessageType::INT);
    return inMessageTypes;
}
std::vector<Enums::MessageType> NodeSourceStream::getOutMessageTypes(){
    std::vector<Enums::MessageType> outMessageTypes;
    outMessageTypes.push_back(Enums::MessageType::PICTURE);

    return outMessageTypes;
}
std::string NodeSourceStream::getDescription(){
    return "Node that fetches camera source";
}
std::string NodeSourceStream::getName(){
    return "NodeSourceStream";
}
Enums::NodeType NodeSourceStream::getType(){
    return Enums::NodeType::NODESOURCESTREAM;
}

void NodeSourceStream::drawNodeParams(){

    //Frame cam state je postavljen...
    
    //Fetch all info i want to know...
    //I bassicaly want to get camState

    ImGui::PushItemWidth(190);

    ImGui::BeginGroup();
    ImGui::InputText("##input", this->streamInputBuffer, IM_ARRAYSIZE(this->streamInputBuffer));

    ImGui::SameLine();
    std::string stateText;

    if(this->frameCamState == Enums::FrameCamState::DISCONNECTED){
        stateText = "connect";
    }
    else if(this->frameCamState == Enums::FrameCamState::CONNECTED){
        stateText = "disconnect";
    }
    else if(this->frameCamState == Enums::FrameCamState::TRYING){
        stateText = "trying...";
    }

    if(this->frameCamState == Enums::FrameCamState::TRYING){
        ImGui::BeginDisabled();
    }
    bool buttonClicked = ImGui::Button(stateText.c_str());
    if(this->frameCamState == Enums::FrameCamState::TRYING){
        ImGui::EndDisabled();
    }

    //Should open stream
    if(buttonClicked && this->frameCamState == Enums::FrameCamState::DISCONNECTED){
        printf("this->frameCamState = Enums::FrameCamState::TRYING;\n");
        this->streamFullName = this->streamInputBuffer;
        this->frameCamState = Enums::FrameCamState::TRYING;
    }
    //Should close stream
    else if(buttonClicked && this->frameCamState == Enums::FrameCamState::CONNECTED){
        this->frameCamState = Enums::FrameCamState::DISCONNECTED;
    }

    

    //Change stream info data...
    if(this->frameCamState == Enums::FrameCamState::CONNECTED){
        ImGui::Text("Nickname: ");
        ImGui::SameLine();
        
        
        if(ImGui::Button(this->openedStream->frameNickName.c_str())){
            ImGui::OpenPopup("Set new nickname");
        }

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Set new nickname", NULL, ImGuiWindowFlags_AlwaysAutoResize)){

            ImGui::InputText("##nicknameBuffer", this->nicknameBuffer, IM_ARRAYSIZE(this->nicknameBuffer));


            ImGui::Separator();

            //static int unused_i = 0;
            //ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

          

            if (ImGui::Button("set", ImVec2(120, 0))) {
                this->openedStream->frameNickName = this->nicknameBuffer;
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::EndPopup();
        }

        ImGui::Text("Resolution: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(70);
        ImGui::DragInt("##rezw",&this->resolution[0],1.0f,1);
        ImGui::SameLine();
        ImGui::Text("x");
        ImGui::SameLine();
        ImGui::DragInt("##rezh",&this->resolution[1],1.0f,1);
        ImGui::SameLine();
        if(ImGui::Button("set"))this->changedres=true;
        ImGui::PopItemWidth();

        ImGui::Text("Lag[ms]: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(50);

        ImGui::DragInt("##streamLagInMilisecond", &this->openedStream->streamLagInMilisecond, 1.0, 0, 5000);
        ImGui::PopItemWidth();

        ImGui::Text("Intrinsic&Distortion: ");
        ImGui::SameLine();
        if(this->intrinsicAndDistortionButtonFlag == false){ // 
            if(ImGui::Button("recalibrate")){
                printf("recalibrate\n");
                this->intrinsicAndDistortionButtonFlag = !this->intrinsicAndDistortionButtonFlag;
            }
            
        }
        else if(this->intrinsicAndDistortionButtonFlag){
            calibrationPointsVector.size();
            std::string str = "stop&calculate [" + std::to_string(calibrationPointsVector.size()) + "]";

            if(ImGui::Button(str.c_str())){
                printf("stop&calculate\n");
                this->intrinsicAndDistortionButtonFlag = !this->intrinsicAndDistortionButtonFlag;
            }
            
        }
        ImGui::PushItemWidth(50);
        ImGui::Text("W: ");
        ImGui::SameLine();
        ImGui::DragInt("##W:",&this->boardW,1.0f,0);
        ImGui::SameLine();
        ImGui::Text("H: ");
        ImGui::SameLine();
        ImGui::DragInt("##H:",&this->boardH,1.0f,0);
        ImGui::PopItemWidth();
        ImGui::Text("squer size: ");
        ImGui::PushItemWidth(60);
        ImGui::SameLine();
        ImGui::DragFloat("mm",&this->boardSQR,0.1f,0.0f);
        ImGui::PopItemWidth();

        ImGui::Text("Undistort: ");
        ImGui::SameLine();
        if(this->openedStream->intrinsicParams.reprojectionError < 0){
            ImGui::BeginDisabled();
        }
        ImGui::Checkbox("##undistort", &this->shouldUndistort);

        if(this->openedStream->intrinsicParams.reprojectionError < 0){
            ImGui::EndDisabled();
        }
                
    }
    

    //Ako je status connected i promijeni se inputText
    //Change state...

    //Stream is closed
    if(this->frameCamState == Enums::FrameCamState::DISCONNECTED){
        if(this->ocupiedDShowNum != -1)
        {
            GlobalParams::getInstance().openedDShowCameras.erase(this->ocupiedDShowNum);
            this->ocupiedDShowNum = -1;
        }
        std::string input_str(this->streamInputBuffer);
        std::transform(input_str.begin(), input_str.end(), input_str.begin(), ::tolower);
        //If frameUrlName is matching any input string but not fully than print that
        std::vector<std::shared_ptr<FrameCam>> camFrames = GlobalParams::getInstance().getCamFrames();
        for(int i = 0; i < camFrames.size(); i++){

            std::string camMemMatch = camFrames[i]->frameUrlName + " -> " + camFrames[i]->frameNickName;
            std::string item_lower = camMemMatch;
            std::transform(item_lower.begin(), item_lower.end(), item_lower.begin(), ::tolower);

            if ((item_lower.find(input_str) != std::string::npos) && (input_str != item_lower))
            {   
                if(camFrames[i]->isConnected){
                    ImGui::BeginDisabled();
                }
                if (ImGui::Selectable((camMemMatch).c_str(), false, 0, ImVec2(camMemMatch.size()*7, 12))){
                    
                    std::strncpy(this->streamInputBuffer, camFrames[i]->frameUrlName.c_str(), sizeof(this->streamInputBuffer) - 1);
                }
                if(camFrames[i]->isConnected){
                    ImGui::EndDisabled();
                }
            }
        }
    }

    
    
    ImGui::PopItemWidth();
    ImGui::EndGroup();
}

void NodeSourceStream::drawNodeWork(){
    std::unique_lock<std::mutex> lock(this->mutex);
    /*
    if(this->frame.empty()){
        this->mutex.unlock();
        return;
    }
    */


    //int n = 20; // Number of spikes
    //int radius_min = 50; // Minimum radius
    //int radius_max = 100;
    //cv::Point center(frame.cols / 2, frame.rows / 2);
    
    cv::Mat frameCpy = this->frame.clone();
    //Util::drawStar(&frameCpy, center, this->n, this->radius_min, this->radius_max, cv::Scalar(0, 0, 255), 100);
    
    

    //std::string text = "[25, 50]";

    // Define the position where the text will be printed
    //cv::Point position(50, 50); // (x, y) coordinates
    // Define the font scale (size)
    //double fontScale = 0.3;

    // Define the color (BGR)
    //ImVec4 colorStar = ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);




    //Util::drawText(frameCpy, text, position, fontScale, colorStar, 1);

    //ACTUAL_PARAMS: myHierarchyEntry&contours  
    //STYLE_PARAMS: areaColor/AreaTransparency  lineColor/lineTransparency/lineWidth textColor/textWidth/textScale
    

    //Util::drawCircle(bool showMiddle, circleMiddle, circleRadius,bool showDiagonalLines, bool showArea, bool showConture);
    //Util::drawRectangle(bool showMiddle, bool showDiagonalLines, bool showArea, bool show)
    //Util::drawObject(myHierarchyEntry, transparency, bool drawContures);
    //Util::drawCenter();
    //Util::drawText(
    //Util::drawRectangleMiddle(bool showMiddle, );
    //Util::DrawBlob(myHierarchyEntry, textures...)
    
    //Util::drawCenter(frameCpy, cv::Point(100, 100), 15, colorStar, 1);
    //Util::drawCenter(frameCpy, cv::Point(101, 100), 15, colorStar, 1);
    
    cv::Mat gray;
    
    if(!frameCpy.empty())
    {
        /*
        cvtColor(frameCpy, gray, cv::COLOR_BGR2GRAY);

        cv::Ptr<cv::AKAZE> orb = cv::AKAZE::create();
 
        // Detect keypoints and compute descriptors
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        orb->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);
        
        //std::cout << "Number of keypoints detected: " << keypoints.size() << std::endl;

        cv::Mat keypointImage;
        cv::drawKeypoints(gray, keypoints, keypointImage, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DEFAULT);
        */
        Util::mat2Texture(frameCpy, this->texture);
        //Util::matToTexture(this->frame, &this->texture, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        ImGui::Image((ImTextureID)(intptr_t)this->texture, ImVec2(this->resolution[0]*GlobalParams::getInstance().getZoom().scaleFactor, this->resolution[1]*GlobalParams::getInstance().getZoom().scaleFactor));
    }
    
    

    
    

    

    //printf("Video texture id = %d\n", videoTexture);
}

void NodeSourceStream::threadLoop(){

    //std::cout << cv::getBuildInformation() << std::endl;


    //cv::VideoCapture cap("rtsp://192.168.1.162/av0_0", cv::CAP_FFMPEG);
    cv::VideoCapture cap;
    /*
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream with GStreamer" << std::endl;
        return;
    }
    else{
        this->streamState = Enums::StreamState::NOT_CONNECTED;
    }
    */
    //this->streamState = Enums::StreamState::NOT_CONNECTED;

    while(1){
        //printf("It is pending\n");
        //cap.set(cv::CAP_PROP_EXPOSURE, this->exposureValue);
        if(this->doneFlag.load() == 1){
            return;
        }
        /*
        if(this->openedStream == nullptr){
            Util::delay(10);
            continue;
        }
        */

        if(this->frameCamState == Enums::FrameCamState::DISCONNECTED){
            //Protect!?
            cap.release();
            if(this->openedStream != nullptr)
            {
                this->openedStream->isConnected = false;
                this->openedStream = nullptr;
            }
            
            continue;
        }
        
        if(this->frameCamState == Enums::FrameCamState::TRYING){
            
            std::vector<std::string> input_name_splited = boost::split(std::vector<std::string>(), this->streamFullName, boost::is_any_of(" \t\n"), boost::token_compress_on);
            printf("input_name_splited = %d\n", input_name_splited.size());
            int number = -1;
            if (!input_name_splited.empty() && boost::all(input_name_splited.back(), ::isdigit)) {
                number = std::stoi(input_name_splited.back()); // Convert to number
                input_name_splited.pop_back();                    // Remove the last token
                this->streamFullName = boost::join(input_name_splited, " ");     // Recreate the original string without the last token
                
                std::cout << "Number: " << number << ", Modified string: " << this->streamFullName << std::endl;
            }

            if(number != -1){
                //Try to open first 10 cams 
                //retrieve which cams are free


                std::unordered_set<int> ocupiedCams = GlobalParams::getInstance().openedDShowCameras;

                if(ocupiedCams.find(number) == ocupiedCams.end()) // means it does not exist -> delete it
                {
                    //try to connect
                    printf("Triing to open\n");
                    cap.open(number, cv::CAP_DSHOW); //OPEN CAMERA DIRECT
                    

                    if(cap.isOpened())
                    {
                        printf("It is opened\n");
                        this->ocupiedDShowNum = number;
                        GlobalParams::getInstance().openedDShowCameras.insert(number);
                        this->streamFullName= std::to_string(number);
                    }
                    
                }

                //printf("Available cams = %d\n", freeCams.size());
                //int number = std::atoi(this->openedStream->frameUrlName.c_str());
                //cap.open(number, cv::CAP_DSHOW);
            }
            else{
                cap.open(this->streamFullName.c_str()); //OPEN CAMERA IP
            }
            

            if (!cap.isOpened()) {
                printf("Failed to connect to stream\n");
                this->frameCamState = Enums::FrameCamState::DISCONNECTED;
                continue;
            }
            else{
                //here save new entry if does not exist
                if(!GlobalParams::getInstance().doesStreamExist(this->streamFullName)){
                    GlobalParams::getInstance().createStream(this->streamFullName, this->streamFullName);
                }
                printf("Stream created\n");
                this->openedStream = GlobalParams::getInstance().openStream(this->streamFullName);

                this->frameCamState = Enums::FrameCamState::CONNECTED;
                this->openedStream->isConnected = true;
            }
        }

        //Here it should be connected and ready...

        

        //std::this_thread::sleep_for(std::chrono::microseconds(1000));
        
        //Util::delay(1);

        cv::Mat newFrame;

        if(cap.read(newFrame)){
            // std::cout << "Native resolution: " << newFrame.cols << "x" << newFrame.rows << std::endl;
            // Only resize if resolution is set and different from native
            if(this->changedres) {
                // cv::resize(newFrame, newFrame, rez);
                cap.set(cv::CAP_PROP_FRAME_WIDTH, this->resolution[0]);
                cap.set(cv::CAP_PROP_FRAME_HEIGHT, this->resolution[1]);
                this->changedres=false;
            }


            pass++;
            if(this->intrinsicAndDistortionButtonFlag && pass % 30 == 0){
                int boardWidth = this->boardW; //PARAM CALIB CALIBRATION 
                int boardHeight = this->boardH;
                cv::Size boardSize = cv::Size(boardWidth, boardHeight);
                printf("newFrame size = %d, %d\n", newFrame.size().width, newFrame.size().height);
                std::vector<cv::Point2f> corners;
                cv::Mat gray;
                cvtColor(newFrame, gray, cv::COLOR_BGR2GRAY);
                bool found = findChessboardCorners(newFrame, boardSize, corners,
                                    cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

                
                if(found){
                    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1);
                    //cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);
                    cv::drawChessboardCorners(newFrame, boardSize, cv::Mat(corners), found);
                    calibrationPointsVector.push_back(corners);
                    
                    //imshow("Chessboard Corners", newFrame);
                    printf("It is found = %d\n", calibrationPointsVector.size());
                    
                }
                
            }
            else if(!this->intrinsicAndDistortionButtonFlag && this->calibrationPointsVector.size() >= this->minCalibPictures){
                printf("Simulating hard work\n");
                int boardWidth = this->boardW; //PARAM CALIB CALIBRATION 
                int boardHeight = this->boardH;
                float squareSize = this->boardSQR; // Square size in millimeters

                cv::Size boardSize(boardWidth, boardHeight);

                std::vector<cv::Point3f> obj;
                for (int i = 0; i < boardHeight; ++i) {
                    for (int j = 0; j < boardWidth; ++j) {
                        obj.push_back(cv::Point3f(j * squareSize, i * squareSize, 0));
                    }
                }


                std::vector<std::vector<cv::Point3f>> objectPoints;
                // Add the collected points to imagePoints
                for (const auto& points : this->calibrationPointsVector) {
                    objectPoints.push_back(obj); // Same object points for each image
                }

                cv::Mat cameraMatrix, distCoeffs;
                std::vector<cv::Mat> rvecs, tvecs;

                // Calibrate the camera
                printf("About to calibrate, %d, %d\n", this->calibrationPointsVector.size(), objectPoints.size());

                double rms = cv::calibrateCamera(objectPoints, this->calibrationPointsVector, boardSize, cameraMatrix, distCoeffs, rvecs, tvecs);

                Structs::IntrinsicCamParams intrinsicParams;
                intrinsicParams.reprojectionError = rms;
                

                std::cout << "Re-projection error reported by calibrateCamera: " << rms << std::endl;
                std::cout << "Camera Matrix: " << std::endl << cameraMatrix << std::endl;
                std::cout << "Distortion Coefficients: " << std::endl << distCoeffs << std::endl;


                this->openedStream->intrinsicParams.reprojectionError = rms;
                this->openedStream->intrinsicParams.intrinsicMatrix = cameraMatrix;
                this->openedStream->intrinsicParams.distortionCoef = distCoeffs;




                //Util::delay(5000);
                this->calibrationPointsVector.clear();
            }
            
            if(this->shouldUndistort){
                cv::Mat tempImage; // Temporary image to store the undistorted result
                cv::undistort(newFrame, tempImage, this->openedStream->intrinsicParams.intrinsicMatrix, this->openedStream->intrinsicParams.distortionCoef);
                newFrame = tempImage;
            }

            
            std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::make_shared<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(this->openedStream ,Enums::MessageType::PICTURE);

            {
                std::unique_lock<std::mutex> lock(this->mutex);
                //printf("Mat size = %d, %d\n", this->frame.size().width, this->frame.size().height);
                this->frame = newFrame.clone();
                
            }


            cv::Mat whiteGrayscaleImage = cv::Mat::ones(newFrame.size(), CV_8UC1) * 255;
            msg->data = std::make_shared<std::pair<cv::Mat, cv::Mat>>(std::make_pair(newFrame, whiteGrayscaleImage));
            msg->startLagDuration = msg->camOrigin->streamLagInMilisecond;
            msg->addPhase("streamSource");

            this->sendAll(msg);

            
            
        }
        
        //printf("BEF, Video texture = %d\n", this->videoTexture);
        //Util::matToTexture(frame, &textureID, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        //printf("AFT, Video texture = %d\n", frame.cols);
        

        //Create new int message
        //std::shared_ptr<Message<int>> msg = std::make_shared<Message<int>>(Enums::MessageType::INT);

        //msg->addPhase("streamSource");




        //this->sendAll(msg);
        //printf("Node stream working\n");
    }
}

void NodeSourceStream::connectionAdded(int connectorId, int connectionId){
    this->connectorsStrategyDynamicConnectionAdded(Enums::ConnectorDirection::OUT_CONNECTOR , connectorId, connectionId);
}

void NodeSourceStream::connectionRemoved(int connectorId, int connectionId){
    this->connectorsStrategyDynamicConnectionRemoved(Enums::ConnectorDirection::OUT_CONNECTOR, connectorId, connectionId);
}

void NodeSourceStream::recieve(std::shared_ptr<MessageBase> message, int connectorId) {
    std::shared_ptr<Message<int>> msg = std::dynamic_pointer_cast<Message<int>>(message);
    this->openedStream->streamLagInMilisecond=msg->data;
    printf("NodeSourceStream recieved %d\n", msg->data);
}

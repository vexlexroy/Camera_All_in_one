#include "MessageBase.hpp"
#include "Util.hpp"
#include "FrameCam.hpp"

#include <vector>
#include <windows.h>
#include <stdio.h>


MessageBase::MessageBase(std::shared_ptr<FrameCam> camOrigin, Enums::MessageType messageType){
    this->messageType = messageType;
    this->camOrigin = camOrigin;
    //printf("+MessageBase\n"); Isključujem namjerno jer će se stvarati puno ovakvih poruka
}

MessageBase::~MessageBase(){
    //printf("-MessageBase\n"); //Isključujem namjerno jer će se stvarati puno ovakvih poruka
}
MessageBase::MessageBase(Enums::MessageType messageType, std::shared_ptr<MessageBase> oldMessage): MessageBase(oldMessage->camOrigin, messageType){
    this->timestamps = oldMessage->timestamps;
    this->startLagDuration = oldMessage->startLagDuration;
}


void MessageBase::addPhase(std::string newPhaseString){
    std::pair<std::string, std::vector<std::pair<std::string, long long int>>> newPhaseTimestamp;
    newPhaseTimestamp.first = newPhaseString;

    this->timestamps.push_back(newPhaseTimestamp);
    this->addTimestamp(newPhaseString);
}

void MessageBase::addTimestamp(std::string timestampName){

    if(this->timestamps.size() == 0){
        printf("WARNING, can not add timestamp because Phase is not added\n");
        return;
    }

    std::pair<std::string, long long int> newTimestamp;
    newTimestamp.first = timestampName;
    newTimestamp.second = Util::timeSinceEpochMicroseconds();
    this->timestamps.back().second.push_back(newTimestamp);
}


long long int MessageBase::getBaseDelay(){
    //get duration of currentTimestamp and first timestamp if exists...
    if(this->timestamps.front().second.size() == 0){
        printf("Warning, not imestamps in first message history\n");
        return 0;
    }


    long long int firstTimestamp = this->timestamps.front().second.front().second;
    long long int currentTimestamp = Util::timeSinceEpochMicroseconds();

    //substract current timestamp with first recorded timestamp and add duration
    return this->startLagDuration*1000 + (currentTimestamp - firstTimestamp);
}

long long int MessageBase::getBaseTimestamp(){
    //get base delay and substract from current time...
    long long int baseDelay = this->getBaseDelay();
    return Util::timeSinceEpochMicroseconds() - baseDelay;
}

void MessageBase::printHistory(){
    long long int baseTimeStamp = this->getBaseTimestamp();
    long long int overallDuration = this->getBaseDelay();

    


    printf("%s | %lldus | [START", Util::printHumanReadableFormat(baseTimeStamp).c_str(), overallDuration);

    long long int lastTimestamp = baseTimeStamp;
    for(int i = 0; i < this->timestamps.size(); i++){
        std::pair<std::string, std::vector<std::pair<std::string, long long int>>> phases = this->timestamps[i];

        
        for(int j = 0; j < phases.second.size(); j++){
            long long int timestamp = phases.second[j].second;
            std::string timestampName = phases.second[j].first;

            long long int duration = timestamp - lastTimestamp;
            float percentage = 100.0*duration/overallDuration;


            if(j == 0){
                printf("] > %lld > [%s", duration, phases.first.c_str());
            }
            else{
                printf(">%lld>%s", duration, timestampName.c_str());
            }
            lastTimestamp = timestamp;
        }
    }

    printf("] > %d > [END]\n", baseTimeStamp + overallDuration - lastTimestamp);
}
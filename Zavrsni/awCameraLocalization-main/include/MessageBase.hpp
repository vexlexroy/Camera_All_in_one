#pragma once

#include "Enums.hpp"
#include "FrameCam.hpp"

#include <vector>
#include <memory>
#include <iostream>

class MessageBase{
    public:
        long long int startLagDuration = 0;
        std::vector<std::pair<std::string, std::vector<std::pair<std::string, long long int>>>> timestamps;
        Enums::MessageType messageType;
        std::shared_ptr<FrameCam> camOrigin;


        //If this is first message in chain...
        MessageBase(std::shared_ptr<FrameCam> camOrigin ,Enums::MessageType messageType);
        ~MessageBase();


        //If there was previous message
        MessageBase(Enums::MessageType messageType, std::shared_ptr<MessageBase> previousMessage);
        
        virtual std::shared_ptr<MessageBase> clone(std::shared_ptr<MessageBase> originalMessage) = 0;

        virtual void addPhase(std::string newPhaseName);

        //some phase must be started...
        void addTimestamp(std::string timestampName);
        
        //From current timestamp calculates how much time passed since message creation
        long long int getBaseDelay();

        long long int getBaseTimestamp();

        void printHistory();
};
#pragma once

#include "MessageBase.hpp"
#include <memory>

template<typename T>
class Message: public MessageBase{
    public:
        T data;

        Message(std::shared_ptr<FrameCam> camOrigin, Enums::MessageType messageType);
        Message(Enums::MessageType messageType, std::shared_ptr<MessageBase> previousMessage);
        
        std::shared_ptr<MessageBase> clone(std::shared_ptr<MessageBase> originalMessage) override;
        
};


#include "Message.tpp"
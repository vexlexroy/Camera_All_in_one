#include "Message.hpp"
#include "Enums.hpp"


template<typename T>
Message<T>::Message(std::shared_ptr<FrameCam> camOrigin, Enums::MessageType messageType) : MessageBase(camOrigin, messageType) {

}

template<typename T>
Message<T>::Message(Enums::MessageType messageType, std::shared_ptr<MessageBase> previousMessage) : MessageBase(messageType, previousMessage) {}

template<typename T>
std::shared_ptr<MessageBase> Message<T>::clone(std::shared_ptr<MessageBase> originalMessage){

    //create msg of same type of this msg, bassicaly copy data
    if(originalMessage->messageType == Enums::MessageType::PICTURE){
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msg = std::dynamic_pointer_cast<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(originalMessage);

        
        std::shared_ptr<std::pair<cv::Mat, cv::Mat>> clonedData = std::make_shared<std::pair<cv::Mat, cv::Mat>>(std::make_pair<cv::Mat, cv::Mat>(msg->data->first.clone(), msg->data->second.clone()));
        std::shared_ptr<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>> msgCpy = std::make_shared<Message<std::shared_ptr<std::pair<cv::Mat, cv::Mat>>>>(Enums::MessageType::PICTURE , originalMessage);
        msgCpy->data = clonedData;
        return msgCpy;
    }
    //dodati za ostale tipove ako treba

    return originalMessage;
}

#include "Connection.hpp"
#include "ConnectionBase.hpp"

#include <chrono>
#include <atomic>

template<typename T>
Connection<T>::Connection(int uniqueId, std::shared_ptr<ConnectorBase> connectorIn, std::shared_ptr<ConnectorBase> connectorOut) : ConnectionBase(uniqueId, connectorIn, connectorOut){
    this->thread = std::thread(&Connection<T>::threadLoop, this);
}



template<typename T>
void Connection<T>::threadLoop(){
    while(true){
        std::unique_lock<std::mutex> lock(this->mutex);
        

        while(this->messageQueue.size() == 0){
            this->conditionVariable.wait_for(lock, std::chrono::seconds(1));
            //printf("This is other thread working,  %d\n", this->doneFlag.load());
            
            if(this->doneFlag.load() != 0){
                //printf("Condition met, stoping thread\n");
                return;
            }
        }

        
        std::shared_ptr<MessageBase> lastMessage;
        while(!this->messageQueue.empty()){
            lastMessage = this->messageQueue.front();
            this->messageQueue.pop();
        }

        //printf("There is new message, %lld\n", lastMessage->startLagDuration);
        this->connectorIn->recieve(lastMessage);

    }
}

template<typename T>
void Connection<T>::send(std::shared_ptr<MessageBase> msg){
    //printf("Going to send msg...\n");

    //Now put data to this->messageQueue, and trigger loop

    this->messageQueue.push(msg);
    this->conditionVariable.notify_one(); // trigger condition variable
    //printf("Type of T: %s\n", typeid(T).name());
}
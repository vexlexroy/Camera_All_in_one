#include "ConnectionBase.hpp"
#include "ConnectorBase.hpp"

#include <memory>
#include <imnodes.h>

ConnectionBase::ConnectionBase(int uniqueId, std::shared_ptr<ConnectorBase> connectorIn, std::shared_ptr<ConnectorBase> connectorOut) {
    this->id = uniqueId;
    this->connectorIn = connectorIn;
    this->connectorOut = connectorOut;
    
    this->doneFlag = 0;

    printf("+ConnectionBase(ID=%d, startConnId=%d, encConnId=%d)\n", this->id, this->connectorOut->connectorId, this->connectorIn->connectorId);
}
ConnectionBase::~ConnectionBase() {
    
    //std::lock_guard<std::mutex> lock(this->mutex);
    
    this->doneFlag = 1;
    
    this->conditionVariable.notify_one();
    if(this->thread.joinable()){
        printf("Joining thread\n");
        this->thread.join();
    }

    printf("-ConnectionBase(ID=%d, startConnId=%d, encConnId=%d)\n", this->id, this->connectorOut->connectorId, this->connectorIn->connectorId);
    

    //Notify that connection(this class) is removed
    this->connectorIn->connectionRemoved(this->id);
    this->connectorOut->connectionRemoved(this->id);
}

void ConnectionBase::drawConnection(){

    ImNodes::Link(this->id, this->connectorOut->connectorId, this->connectorIn->connectorId);


}


void ConnectionBase::threadLoop(){}

#include "NodeSource.hpp"


void NodeSource::startThread(){
    this->thread = std::thread(&NodeSource::threadLoop, this);
}

NodeSource::NodeSource(int uniqueId): NodeBase(uniqueId){
    this->doneFlag = 0;
}

NodeSource::~NodeSource(){
    this->doneFlag = 1;
    
    if(this->thread.joinable()){
        this->thread.join();
    }
}
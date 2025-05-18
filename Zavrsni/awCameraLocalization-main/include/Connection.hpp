#pragma once
#include "ConnectionBase.hpp"
#include "Message.hpp"
#include "MessageBase.hpp"

#include <vector>

template<typename T>
class Connection: public ConnectionBase{
public:
    std::queue<std::shared_ptr<MessageBase>> messageQueue;
    

    Connection(int uniqueId, std::shared_ptr<ConnectorBase> connectorIn, std::shared_ptr<ConnectorBase> connectorOut);

    void threadLoop() override;
    void send(std::shared_ptr<MessageBase> msg) override;
};

#include "Connection.tpp"
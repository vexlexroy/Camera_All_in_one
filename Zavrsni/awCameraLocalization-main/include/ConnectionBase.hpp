#pragma once

#include "MessageBase.hpp"

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>


class ConnectorBase;

class ConnectionBase {
public:
    int id;
    std::shared_ptr<ConnectorBase> connectorIn;
    std::shared_ptr<ConnectorBase> connectorOut;

    std::thread thread;
    std::condition_variable conditionVariable;
    std::mutex mutex;
    std::atomic<int> doneFlag;
    

    ConnectionBase(int uniqueId, std::shared_ptr<ConnectorBase> connectorIn, std::shared_ptr<ConnectorBase> connectorOut);
    ~ConnectionBase();

    virtual void drawConnection();

    virtual void send(std::shared_ptr<MessageBase> msg) = 0;

    virtual void threadLoop();

};

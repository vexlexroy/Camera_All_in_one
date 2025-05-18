#pragma once
#include "NodeBase.hpp"

#include <thread>
#include <memory>
#include <mutex>

class NodeSource: public NodeBase{
    public:
        std::thread thread;
        std::atomic<int> doneFlag;
        std::mutex mutex;


        NodeSource(int id);
        ~NodeSource();

        void startThread();
        virtual void threadLoop() = 0;
};
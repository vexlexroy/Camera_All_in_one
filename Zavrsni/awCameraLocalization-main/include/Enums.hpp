#pragma once

namespace Enums{
    enum class NodeType{
        NODETEST,
        NODETEST2,
        NODESOURCESTREAM,
        NODEINFLATE,
        NODEBACKGROUNDSUBSTRACTION,
        NODEBLOBCREATOR,
        NODEBLOBGROUPER,
        NODEMANUALEXTRINSIC,
        NODEDELAYMESURMENTE
    };

    enum class ConnectorDirection{
        IN_CONNECTOR,
        OUT_CONNECTOR,
        ALL
    };

    //for every added MessageType, Factory must implement its ConnectorIn, ConnectorOut and Connection
    enum class MessageType{
        INT,
        FLOAT,
        PICTURE,
        BLOB,
        BLOBGROUP,
        ALL
    };


    enum class ConnectorState{
        CONNECTED,
        UNCONNECTED,
        ALL
    };

    enum class FrameCamState{
        TRYING,
        CONNECTED,
        DISCONNECTED
    };

    enum class PairStatus{
        UNUSED,
        CALIBRATION,
        ESTIMATION
    };
}
#pragma once
#include "FrameBase.hpp"

class FrameCustom : public FrameBase{
public:

    std::string cam1Nickname;
    std::string cam2Nickname;

    FrameCustom(std::string nickname, std::string cam1Nickname, std::string cam2Nickname);

private:

};

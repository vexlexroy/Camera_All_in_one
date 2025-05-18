#include "FrameCustom.hpp"


FrameCustom::FrameCustom(std::string nickname, std::string cam1Nickname, std::string cam2Nickname): FrameBase(nickname){
    this->cam1Nickname = cam1Nickname;
    this->cam2Nickname = cam2Nickname;
}

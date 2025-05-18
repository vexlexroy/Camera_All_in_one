#pragma once
#include <stdio.h>
#include <string>

class Class_base{
    public:
        std::string name;
        virtual void printName(){
            printf("%s\n", this->name.c_str());
        }
};
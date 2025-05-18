#pragma once
#include <iostream>
#include "Class_base.hpp"
#include "Class_2.hpp"

template<typename T>
class Class_2;

template<typename T>
class Class_1: public Class_base{
    public:
        T dataSpecific;
        std::shared_ptr<Class_base> sharedPtr;

        Class_1(std::string name, T dataSpecific){
            this->dataSpecific = dataSpecific;
            this->name = name;
        }
        ~Class_1(){
            
        }

        void printSpecific(){
            std::cout << dataSpecific << std::endl;
        }
};
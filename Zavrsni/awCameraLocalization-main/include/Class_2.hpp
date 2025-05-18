#pragma once
#include <iostream>
#include "Class_base.hpp"
#include "Class_1.hpp"


template<typename T>
class Class_1;

template<typename T>
class Class_2: public Class_base{
    public:
        T dataSpecific;
        std::shared_ptr<Class_base> sharedPtr;

        Class_2(std::string name, T dataSpecific){
            this->dataSpecific = dataSpecific;
            this->name = name;
        }
        ~Class_2(){
            
        }
        
        void printSpecific(){
            std::cout << "2>" << dataSpecific << std::endl;
        }
};
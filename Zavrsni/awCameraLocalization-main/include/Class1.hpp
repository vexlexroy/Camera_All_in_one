// Class1.hpp
#ifndef CLASS1_HPP
#define CLASS1_HPP

#include <memory>
#include "Class2.hpp" // Include Class2 header for the shared_ptr declaration
#include <stdio.h>

template<typename T>
class Class2; // Forward declaration for Class2

template<typename T>
class Class1 {   
public:
    std::shared_ptr<Class2<T>> instanceOfClass2;
    T data;
    Class1(T data){this->data = data;}
};

#endif // CLASS1_HPP

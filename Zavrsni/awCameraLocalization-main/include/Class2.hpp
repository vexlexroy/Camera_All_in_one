// Class2.hpp
#ifndef CLASS2_HPP
#define CLASS2_HPP

#include <memory>
#include "Class1.hpp" // Include Class1 header for the shared_ptr declaration

template<typename T>
class Class1; // Forward declaration for Class1

template<typename T>
class Class2 {

public:
    std::shared_ptr<Class1<T>> instanceOfClass1;
    T data;
    Class2(T data){this->data = data;}

};

#endif // CLASS2_HPP

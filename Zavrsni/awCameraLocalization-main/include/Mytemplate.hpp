#pragma once

#include <iostream>

template<typename T>
class MyTemplate {
public:
    MyTemplate(const T& value) : data(value) {}
    void printValue() const {
        std::cout << "Value: " << data << std::endl;
    }
    void printValueTwice(const MyTemplate<T>& obj) {
        obj.printValue();
        obj.printValue();
    }
private:
    T data;
};




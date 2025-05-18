// TemplateExample.hpp

#pragma once

template<typename T>
class TemplateExample {
private:
    T data;

public:
    // Constructor declaration
    TemplateExample(T value);

    // Function declarations
    void setData(T value);
    T getData() const;
};

// Include the template function definitions from the .tpp file
#include "TemplateExample.tpp"

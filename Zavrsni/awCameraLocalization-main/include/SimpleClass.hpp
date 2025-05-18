#pragma once

class SimpleClass {
public:
    // Constructor
    SimpleClass(int value);

    // Member function
    void setValue(int value);

    // Getter function
    int getValue() const;

private:
    int data;
};

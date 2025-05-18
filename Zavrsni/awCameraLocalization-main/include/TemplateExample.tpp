// TemplateExample.tpp
//#include "TemplateExample.hpp"

template<typename T>
TemplateExample<T>::TemplateExample(T value) : data(value) {}

template<typename T>
void TemplateExample<T>::setData(T value) {
    data = value;
}

template<typename T>
T TemplateExample<T>::getData() const {
    return data;
}

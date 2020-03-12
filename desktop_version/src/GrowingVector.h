#ifndef GROWINGVECTOR_H
#define GROWINGVECTOR_H

#include <vector>

template<typename T>
class growing_vector : public std::vector<T> {
    public:
    template<typename Dummy = void>
    growing_vector<T>(const std::vector<T>& v) : std::vector<T>(v) {}
    using std::vector<T>::vector;

    template<typename T2 = T>
    T2& operator[](typename std::vector<T2>::size_type index) {
        typename std::vector<T>::size_type needed_size = index + 1;
        if (this->size() < needed_size) {
            this->resize(needed_size);
        }
        return std::vector<T>::operator[](index);
    }
};

#endif

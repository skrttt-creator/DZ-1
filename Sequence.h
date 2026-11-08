#pragma once
#include <cstddef>

template <class T>
class Sequence {
public:
    virtual ~Sequence() = default;

    virtual const T& Get(size_t index) const = 0;
    virtual T& Get(size_t index) = 0;
    virtual size_t GetLength() const = 0;
    virtual void Append(const T& item) = 0;

    virtual T& operator[](size_t index) = 0;
    virtual const T& operator[](size_t index) const = 0;
};
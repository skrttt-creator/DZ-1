#pragma once
#include "ArraySequence.h"

template <class T>
ArraySequence<T>::ArraySequence()
    : data(UnqPtr<T>::MakeArray(4)), length(0), capacity(4) {
}

template <class T>
ArraySequence<T>::ArraySequence(size_t initialCapacity)
    : data(UnqPtr<T>::MakeArray(initialCapacity > 0 ? initialCapacity : 1)),
      length(0),
      capacity(initialCapacity > 0 ? initialCapacity : 1) {
}

template <class T>
ArraySequence<T>::ArraySequence(const T* items, size_t count)
    : data(UnqPtr<T>::MakeArray(count > 0 ? count : 1)),
      length(count),
      capacity(count > 0 ? count : 1) {
    if (items == nullptr && count > 0) {
        throw InvalidArgument("ArraySequence: items pointer is null");
    }
    for (size_t i = 0; i < count; ++i) {
        data[i] = items[i];
    }
}

template <class T>
ArraySequence<T>::ArraySequence(const ArraySequence<T>& other)
    : data(other.data), length(other.length), capacity(other.capacity) {
}

template <class T>
ArraySequence<T>::ArraySequence(ArraySequence<T>&& other) noexcept
    : data(std::move(other.data)), length(other.length), capacity(other.capacity) {
    other.length = 0;
    other.capacity = 0;
}

template <class T>
ArraySequence<T>& ArraySequence<T>::operator=(const ArraySequence<T>& other) {
    if (this != &other) {
        data = other.data;
        length = other.length;
        capacity = other.capacity;
    }
    return *this;
}

template <class T>
ArraySequence<T>& ArraySequence<T>::operator=(ArraySequence<T>&& other) noexcept {
    if (this != &other) {
        data = std::move(other.data);
        length = other.length;
        capacity = other.capacity;
        other.length = 0;
        other.capacity = 0;
    }
    return *this;
}

template <class T>
void ArraySequence<T>::EnsureCapacity(size_t minCapacity) {
    if (minCapacity > capacity) {
        size_t newCap = capacity > 0 ? capacity * 2 : 4;
        if (newCap < minCapacity) {
            newCap = minCapacity;
        }
        UnqPtr<T> newData = UnqPtr<T>::MakeArray(newCap);
        for (size_t i = 0; i < length; ++i) {
            newData[i] = data[i];
        }
        data = std::move(newData);
        capacity = newCap;
    }
}

template <class T>
const T& ArraySequence<T>::Get(size_t index) const {
    if (index >= length) {
        throw IndexOutOfRange("ArraySequence::Get: Index out of range");
    }
    return data[index];
}

template <class T>
T& ArraySequence<T>::Get(size_t index) {
    if (index >= length) {
        throw IndexOutOfRange("ArraySequence::Get: Index out of range");
    }
    return data[index];
}

template <class T>
const T& ArraySequence<T>::GetFirst() const {
    if (length == 0) {
        throw IndexOutOfRange("ArraySequence::GetFirst: Sequence is empty");
    }
    return data[0];
}

template <class T>
const T& ArraySequence<T>::GetLast() const {
    if (length == 0) {
        throw IndexOutOfRange("ArraySequence::GetLast: Sequence is empty");
    }
    return data[length - 1];
}

template <class T>
size_t ArraySequence<T>::GetLength() const {
    return length;
}

template <class T>
void ArraySequence<T>::Append(const T& item) {
    EnsureCapacity(length + 1);
    data[length] = item;
    length++;
}

template <class T>
void ArraySequence<T>::Prepend(const T& item) {
    EnsureCapacity(length + 1);
    for (size_t i = length; i > 0; --i) {
        data[i] = data[i - 1];
    }
    data[0] = item;
    length++;
}

template <class T>
void ArraySequence<T>::InsertAt(const T& item, size_t index) {
    if (index > length) {
        throw IndexOutOfRange("ArraySequence::InsertAt: Index out of range");
    }
    EnsureCapacity(length + 1);
    for (size_t i = length; i > index; --i) {
        data[i] = data[i - 1];
    }
    data[index] = item;
    length++;
}

template <class T>
T& ArraySequence<T>::operator[](size_t index) {
    return Get(index);
}

template <class T>
const T& ArraySequence<T>::operator[](size_t index) const {
    return Get(index);
}
#pragma once
#include "UnqPtr.h"



template <class T>
UnqPtr<T>::UnqPtr() : ptr(nullptr) {}

template <class T>
UnqPtr<T>::UnqPtr(T* rawPtr) : ptr(rawPtr) {}

template <class T>
UnqPtr<T>::UnqPtr(UnqPtr<T>&& other) noexcept : ptr(other.ptr) {
    other.ptr = nullptr;
}

template <class T>
template <class U, typename>
UnqPtr<T>::UnqPtr(UnqPtr<U>&& other) noexcept : ptr(static_cast<T*>(other.ptr)) {
    other.ptr = nullptr;
}

template <class T>
UnqPtr<T>::~UnqPtr() {
    delete ptr;
}

template <class T>
UnqPtr<T>& UnqPtr<T>::operator=(UnqPtr<T>&& other) noexcept {
    if (this != &other) {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    return *this;
}

template <class T>
template <class U, typename>
UnqPtr<T>& UnqPtr<T>::operator=(UnqPtr<U>&& other) noexcept {
    if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
        delete ptr;
        ptr = static_cast<T*>(other.ptr);
        other.ptr = nullptr;
    }
    return *this;
}

template <class T>
T& UnqPtr<T>::operator*() const {
    if (ptr == nullptr) {
        throw NullPointerException("UnqPtr::operator*: Dereferencing null pointer");
    }
    return *ptr;
}

template <class T>
T* UnqPtr<T>::operator->() const {
    if (ptr == nullptr) {
        throw NullPointerException("UnqPtr::operator->: Dereferencing null pointer");
    }
    return ptr;
}

template <class T>
T* UnqPtr<T>::Get() const {
    return ptr;
}

template <class T>
bool UnqPtr<T>::IsNull() const {
    return ptr == nullptr;
}

template <class T>
T* UnqPtr<T>::Release() {
    T* temp = ptr;
    ptr = nullptr;
    return temp;
}

template <class T>
void UnqPtr<T>::Reset(T* newPtr) {
    if (ptr != newPtr) {
        delete ptr;
        ptr = newPtr;
    }
}



template <class T>
UnqPtr<T[]>::UnqPtr() : ptr(nullptr) {}

template <class T>
UnqPtr<T[]>::UnqPtr(T* rawArray) : ptr(rawArray) {}

template <class T>
UnqPtr<T[]>::UnqPtr(UnqPtr<T[]>&& other) noexcept : ptr(other.ptr) {
    other.ptr = nullptr;
}

template <class T>
UnqPtr<T[]>& UnqPtr<T[]>::operator=(UnqPtr<T[]>&& other) noexcept {
    if (this != &other) {
        delete[] ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    return *this;
}

template <class T>
UnqPtr<T[]>::~UnqPtr() {
    delete[] ptr;
}

template <class T>
T& UnqPtr<T[]>::operator[](size_t index) const {
    if (ptr == nullptr) {
        throw NullPointerException("UnqPtr<T[]>::operator[]: Dereferencing null pointer");
    }
    return ptr[index];
}

template <class T>
T* UnqPtr<T[]>::Get() const {
    return ptr;
}

template <class T>
bool UnqPtr<T[]>::IsNull() const {
    return ptr == nullptr;
}

template <class T>
void UnqPtr<T[]>::Reset(T* newArray) {
    if (ptr != newArray) {
        delete[] ptr;
        ptr = newArray;
    }
}
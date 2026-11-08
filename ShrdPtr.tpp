#pragma once
#include "ShrdPtr.h"

template <class T>
ShrdPtr<T>::ShrdPtr() : ptr(nullptr), referenceCount(nullptr) {}

template <class T>
ShrdPtr<T>::ShrdPtr(T* rawPtr)
    : ptr(rawPtr), referenceCount(rawPtr ? new size_t(1) : nullptr) {}

template <class T>
ShrdPtr<T>::ShrdPtr(const ShrdPtr<T>& other)
    : ptr(other.ptr), referenceCount(other.referenceCount) {
    if (referenceCount != nullptr) {
        (*referenceCount)++;
    }
}

template <class T>
template <class U, typename>
ShrdPtr<T>::ShrdPtr(const ShrdPtr<U>& other)
    : ptr(static_cast<T*>(other.ptr)), referenceCount(other.referenceCount) {
    if (referenceCount != nullptr) {
        (*referenceCount)++;
    }
}

template <class T>
ShrdPtr<T>::ShrdPtr(ShrdPtr<T>&& other) noexcept
    : ptr(other.ptr), referenceCount(other.referenceCount) {
    other.ptr = nullptr;
    other.referenceCount = nullptr;
}

template <class T>
ShrdPtr<T>::~ShrdPtr() {
    Release();
}

template <class T>
void ShrdPtr<T>::Release() {
    if (referenceCount != nullptr) {
        (*referenceCount)--;
        if (*referenceCount == 0) {
            delete ptr;
            delete referenceCount;
        }
        ptr = nullptr;
        referenceCount = nullptr;
    }
}

template <class T>
ShrdPtr<T>& ShrdPtr<T>::operator=(const ShrdPtr<T>& other) {
    if (this != &other) {
        Release();
        ptr = other.ptr;
        referenceCount = other.referenceCount;
        if (referenceCount != nullptr) {
            (*referenceCount)++;
        }
    }
    return *this;
}

template <class T>
template <class U, typename>
ShrdPtr<T>& ShrdPtr<T>::operator=(const ShrdPtr<U>& other) {
    if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
        Release();
        ptr = static_cast<T*>(other.ptr);
        referenceCount = other.referenceCount;
        if (referenceCount != nullptr) {
            (*referenceCount)++;
        }
    }
    return *this;
}

template <class T>
ShrdPtr<T>& ShrdPtr<T>::operator=(ShrdPtr<T>&& other) noexcept {
    if (this != &other) {
        Release();
        ptr = other.ptr;
        referenceCount = other.referenceCount;
        other.ptr = nullptr;
        other.referenceCount = nullptr;
    }
    return *this;
}

template <class T>
T& ShrdPtr<T>::operator*() const {
    if (ptr == nullptr) {
        throw NullPointerException("ShrdPtr::operator*: Dereferencing null pointer");
    }
    return *ptr;
}

template <class T>
T* ShrdPtr<T>::operator->() const {
    if (ptr == nullptr) {
        throw NullPointerException("ShrdPtr::operator->: Dereferencing null pointer");
    }
    return ptr;
}

template <class T>
T* ShrdPtr<T>::Get() const {
    return ptr;
}

template <class T>
size_t ShrdPtr<T>::UseCount() const {
    return referenceCount != nullptr ? *referenceCount : 0;
}

template <class T>
bool ShrdPtr<T>::IsNull() const {
    return ptr == nullptr;
}

template <class T>
void ShrdPtr<T>::Reset(T* newPtr) {
    if (ptr != newPtr) {
        Release();
        if (newPtr != nullptr) {
            ptr = newPtr;
            referenceCount = new size_t(1);
        }
    }
}
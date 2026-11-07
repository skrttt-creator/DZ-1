#pragma once
#include "UnqPtr.h"

template <class T>
UnqPtr<T>::UnqPtr()
    : ptr(nullptr), isArray(false), arraySize(0), cloner(nullptr) {
}

template <class T>
UnqPtr<T>::UnqPtr(T* rawPtr)
    : ptr(rawPtr), isArray(false), arraySize(rawPtr ? 1 : 0), cloner(rawPtr ? &CloneSingle<T> : nullptr) {
    if (rawPtr != nullptr) {
        MemoryTracker::OnAlloc();
    }
}

template <class T>
UnqPtr<T>::UnqPtr(T* rawPtr, bool arr, size_t count, T* (*cloneFn)(const T*, size_t))
    : ptr(rawPtr), isArray(arr), arraySize(count), cloner(cloneFn) {
    if (rawPtr != nullptr) {
        MemoryTracker::OnAlloc();
    }
}

template <class T>
UnqPtr<T>::UnqPtr(const UnqPtr<T>& other)
    : ptr(nullptr), isArray(other.isArray), arraySize(other.arraySize), cloner(other.cloner) {
    if (other.ptr != nullptr && other.cloner != nullptr) {
        ptr = other.cloner(other.ptr, other.arraySize);
        MemoryTracker::OnAlloc();
    }
}

template <class T>
UnqPtr<T>::UnqPtr(UnqPtr<T>&& other) noexcept
    : ptr(other.ptr), isArray(other.isArray), arraySize(other.arraySize), cloner(other.cloner) {
    other.ptr = nullptr;
    other.isArray = false;
    other.arraySize = 0;
    other.cloner = nullptr;
}

template <class T>
template <class U, typename>
UnqPtr<T>::UnqPtr(UnqPtr<U>&& other) noexcept
    : ptr(static_cast<T*>(other.ptr)), isArray(other.isArray), arraySize(other.arraySize), cloner(nullptr) {
    if (ptr != nullptr && !isArray) {
        cloner = &CloneSingle<U>;
    }
    other.ptr = nullptr;
    other.isArray = false;
    other.arraySize = 0;
    other.cloner = nullptr;
}

template <class T>
template <class U, typename>
UnqPtr<T>::UnqPtr(const UnqPtr<U>& other)
    : ptr(nullptr), isArray(false), arraySize(other.arraySize), cloner(&CloneSingle<U>) {
    if (other.ptr != nullptr) {
        if constexpr (std::is_copy_constructible_v<U>) {
            ptr = static_cast<T*>(new U(*other.ptr));
            MemoryTracker::OnAlloc();
        } else {
            throw InvalidArgument("UnqPtr: Subtype U is not copy-constructible");
        }
    }
}

template <class T>
UnqPtr<T>::~UnqPtr() {
    CleanUp();
}

template <class T>
UnqPtr<T> UnqPtr<T>::MakeArray(size_t count) {
    if (count == 0) {
        return UnqPtr<T>();
    }
    T* rawArr = new T[count]();
    return UnqPtr<T>(rawArr, true, count, &CloneArray);
}

template <class T>
void UnqPtr<T>::CleanUp() {
    if (ptr != nullptr) {
        MemoryTracker::OnFree();
        if (isArray) {
            delete[] ptr;
        } else {
            delete ptr;
        }
        ptr = nullptr;
        isArray = false;
        arraySize = 0;
        cloner = nullptr;
    }
}

template <class T>
UnqPtr<T>& UnqPtr<T>::operator=(const UnqPtr<T>& other) {
    if (this != &other) {
        CleanUp();
        isArray = other.isArray;
        arraySize = other.arraySize;
        cloner = other.cloner;
        if (other.ptr != nullptr && other.cloner != nullptr) {
            ptr = other.cloner(other.ptr, other.arraySize);
            MemoryTracker::OnAlloc();
        }
    }
    return *this;
}

template <class T>
UnqPtr<T>& UnqPtr<T>::operator=(UnqPtr<T>&& other) noexcept {
    if (this != &other) {
        CleanUp();
        ptr = other.ptr;
        isArray = other.isArray;
        arraySize = other.arraySize;
        cloner = other.cloner;
        other.ptr = nullptr;
        other.isArray = false;
        other.arraySize = 0;
        other.cloner = nullptr;
    }
    return *this;
}

template <class T>
template <class U, typename>
UnqPtr<T>& UnqPtr<T>::operator=(UnqPtr<U>&& other) noexcept {
    if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
        CleanUp();
        ptr = static_cast<T*>(other.ptr);
        isArray = other.isArray;
        arraySize = other.arraySize;
        cloner = (ptr != nullptr && !isArray) ? &CloneSingle<U> : nullptr;
        other.ptr = nullptr;
        other.isArray = false;
        other.arraySize = 0;
        other.cloner = nullptr;
    }
    return *this;
}

template <class T>
UnqPtr<T>& UnqPtr<T>::operator=(std::nullptr_t) {
    CleanUp();
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
T& UnqPtr<T>::operator[](size_t index) const {
    if (ptr == nullptr) {
        throw NullPointerException("UnqPtr::operator[]: Dereferencing null pointer");
    }
    if (isArray && index >= arraySize) {
        throw IndexOutOfRange("UnqPtr::operator[]: Index out of array bounds");
    }
    return ptr[index];
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
UnqPtr<T>::operator bool() const {
    return ptr != nullptr;
}

template <class T>
T* UnqPtr<T>::Release() {
    T* temp = ptr;
    if (ptr != nullptr) {
        MemoryTracker::OnFree();
    }
    ptr = nullptr;
    isArray = false;
    arraySize = 0;
    cloner = nullptr;
    return temp;
}

template <class T>
void UnqPtr<T>::Reset(T* newPtr) {
    if (ptr != newPtr) {
        CleanUp();
        if (newPtr != nullptr) {
            ptr = newPtr;
            isArray = false;
            arraySize = 1;
            cloner = &CloneSingle<T>;
            MemoryTracker::OnAlloc();
        }
    }
}

template <class T>
void UnqPtr<T>::Swap(UnqPtr<T>& other) noexcept {
    T* tmpPtr = ptr;
    bool tmpArr = isArray;
    size_t tmpSize = arraySize;
    auto tmpCloner = cloner;

    ptr = other.ptr;
    isArray = other.isArray;
    arraySize = other.arraySize;
    cloner = other.cloner;

    other.ptr = tmpPtr;
    other.isArray = tmpArr;
    other.arraySize = tmpSize;
    other.cloner = tmpCloner;
}
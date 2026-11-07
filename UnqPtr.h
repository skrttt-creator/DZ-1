#pragma once
#include <cstddef>
#include <type_traits>
#include "Exceptions.h"
#include "MemoryTracker.h"

template <class T>
class UnqPtr {
private:
    T* ptr;
    bool isArray;
    size_t arraySize;
    T* (*cloner)(const T*, size_t);

    template <class U>
    friend class UnqPtr;

    template <class U>
    static T* CloneSingle(const T* src, size_t) {
        if (src == nullptr) {
            return nullptr;
        }
        if constexpr (std::is_copy_constructible_v<U>) {
            return static_cast<T*>(new U(*static_cast<const U*>(src)));
        } else {
            throw InvalidArgument("UnqPtr: Type is not copy-constructible");
        }
    }

    static T* CloneArray(const T* src, size_t count) {
        if (src == nullptr) {
            return nullptr;
        }
        if constexpr (std::is_copy_assignable_v<T> && std::is_default_constructible_v<T>) {
            T* copy = new T[count]();
            for (size_t i = 0; i < count; ++i) {
                copy[i] = src[i];
            }
            return copy;
        } else {
            throw InvalidArgument("UnqPtr: Array element type is not copyable");
        }
    }

    UnqPtr(T* rawPtr, bool arr, size_t count, T* (*cloneFn)(const T*, size_t));
    void CleanUp();

public:
    UnqPtr();
    explicit UnqPtr(T* rawPtr);

    
    UnqPtr(const UnqPtr<T>& other);

    
    UnqPtr(UnqPtr<T>&& other) noexcept;

    
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr(UnqPtr<U>&& other) noexcept;

    
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr(const UnqPtr<U>& other);

    ~UnqPtr();

    
    static UnqPtr<T> MakeArray(size_t count);

    UnqPtr<T>& operator=(const UnqPtr<T>& other);
    UnqPtr<T>& operator=(UnqPtr<T>&& other) noexcept;

    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr<T>& operator=(UnqPtr<U>&& other) noexcept;

    UnqPtr<T>& operator=(std::nullptr_t);

    T& operator*() const;
    T* operator->() const;
    T& operator[](size_t index) const;

    T* Get() const;
    bool IsNull() const;
    explicit operator bool() const;

    T* Release();
    void Reset(T* newPtr = nullptr);
    void Swap(UnqPtr<T>& other) noexcept;
};

#include "UnqPtr.tpp"
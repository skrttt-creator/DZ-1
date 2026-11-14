#pragma once
#include <cstddef>
#include <type_traits>
#include "Exceptions.h"


template <class T>
class UnqPtr {
private:
    T* ptr;

    template <class U>
    friend class UnqPtr;

public:
    UnqPtr();
    explicit UnqPtr(T* rawPtr);

    
    UnqPtr(const UnqPtr<T>&) = delete;
    UnqPtr<T>& operator=(const UnqPtr<T>&) = delete;

    
    UnqPtr(UnqPtr<T>&& other) noexcept;

    
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr(UnqPtr<U>&& other) noexcept;

    ~UnqPtr();

    UnqPtr<T>& operator=(UnqPtr<T>&& other) noexcept;

    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr<T>& operator=(UnqPtr<U>&& other) noexcept;

    T& operator*() const;
    T* operator->() const;

    T* Get() const;
    bool IsNull() const;
    T* Release();
    void Reset(T* newPtr = nullptr);
};


template <class T>
class UnqPtr<T[]> {
private:
    T* ptr;

public:
    UnqPtr();
    explicit UnqPtr(T* rawArray);

    UnqPtr(const UnqPtr<T[]>&) = delete;
    UnqPtr<T[]>& operator=(const UnqPtr<T[]>&) = delete;

    UnqPtr(UnqPtr<T[]>&& other) noexcept;
    UnqPtr<T[]>& operator=(UnqPtr<T[]>&& other) noexcept;

    ~UnqPtr();

    T& operator[](size_t index) const;
    T* Get() const;
    bool IsNull() const;
    void Reset(T* newArray = nullptr);
};

#include "UnqPtr.tpp"
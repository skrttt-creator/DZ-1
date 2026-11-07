#pragma once
#include <cstddef>
#include <type_traits>
#include "Exceptions.h"
#include "MemoryTracker.h"
#include "UnqPtr.h"

template <class T>
class ShrdPtr {
private:
    T* ptr;
    size_t* referenceCount;

    template <class U>
    friend class ShrdPtr;

    void Release();

public:
    ShrdPtr();
    explicit ShrdPtr(T* rawPtr);

    
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    explicit ShrdPtr(UnqPtr<U>&& uniquePtr);

    ShrdPtr(const ShrdPtr<T>& other);

    
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr(const ShrdPtr<U>& other);

    ShrdPtr(ShrdPtr<T>&& other) noexcept;

    ~ShrdPtr();

    ShrdPtr<T>& operator=(const ShrdPtr<T>& other);

    
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr<T>& operator=(const ShrdPtr<U>& other);

    ShrdPtr<T>& operator=(ShrdPtr<T>&& other) noexcept;
    ShrdPtr<T>& operator=(std::nullptr_t);

    T& operator*() const;
    T* operator->() const;

    T* Get() const;
    size_t UseCount() const;
    bool IsNull() const;
    explicit operator bool() const;

    void Reset(T* newPtr = nullptr);

    bool operator==(const ShrdPtr<T>& other) const;
    bool operator!=(const ShrdPtr<T>& other) const;
};

#include "ShrdPtr.tpp"
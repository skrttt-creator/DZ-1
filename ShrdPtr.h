#pragma once
#include <cstddef>
#include <type_traits>
#include "Exceptions.h"

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

    ShrdPtr(const ShrdPtr<T>& other);

    // Поддержка подтипизации при копировании (если U — подкласс T)
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr(const ShrdPtr<U>& other);

    ShrdPtr(ShrdPtr<T>&& other) noexcept;

    ~ShrdPtr();

    ShrdPtr<T>& operator=(const ShrdPtr<T>& other);

    // Поддержка подтипизации при присваивании (если U — подкласс T)
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr<T>& operator=(const ShrdPtr<U>& other);

    ShrdPtr<T>& operator=(ShrdPtr<T>&& other) noexcept;

    T& operator*() const;
    T* operator->() const;

    T* Get() const;
    size_t UseCount() const;
    bool IsNull() const;
    void Reset(T* newPtr = nullptr);
};

#include "ShrdPtr.tpp"
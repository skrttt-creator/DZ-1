#pragma once
#include <cstddef>
#include <type_traits>
#include "Exceptions.h"

// 1. Базовый шаблон UnqPtr<T> для одиночных объектов
template <class T>
class UnqPtr {
private:
    T* ptr;

    template <class U>
    friend class UnqPtr;

public:
    UnqPtr();
    explicit UnqPtr(T* rawPtr);

    // Копирующий конструктор: создаёт глубокую копию объекта (семантика передачи по значению)
    UnqPtr(const UnqPtr<T>& other);

    // Поддержка подтипизации при копировании (если U — подкласс T)
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr(const UnqPtr<U>& other);

    // Перемещающий конструктор (передача владения за O(1))
    UnqPtr(UnqPtr<T>&& other) noexcept;

    // Поддержка подтипизации при перемещении (если U — подкласс T)
    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr(UnqPtr<U>&& other) noexcept;

    ~UnqPtr();

    UnqPtr<T>& operator=(const UnqPtr<T>& other);

    template <class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr<T>& operator=(const UnqPtr<U>& other);

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

// 2. Специализация UnqPtr<T[]> для автоматического управления массивами (delete[])
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
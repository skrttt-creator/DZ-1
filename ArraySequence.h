#pragma once
#include <cstddef>
#include "Sequence.h"
#include "UnqPtr.h"
#include "Exceptions.h"

template <class T>
class ArraySequence : public Sequence<T> {
private:
    UnqPtr<T> data;
    size_t length;
    size_t capacity;

    void EnsureCapacity(size_t minCapacity);

public:
    ArraySequence();
    explicit ArraySequence(size_t initialCapacity);
    ArraySequence(const T* items, size_t count);
    ArraySequence(const ArraySequence<T>& other);
    ArraySequence(ArraySequence<T>&& other) noexcept;

    ArraySequence<T>& operator=(const ArraySequence<T>& other);
    ArraySequence<T>& operator=(ArraySequence<T>&& other) noexcept;

    ~ArraySequence() override = default;

    const T& Get(size_t index) const override;
    T& Get(size_t index) override;
    const T& GetFirst() const override;
    const T& GetLast() const override;
    size_t GetLength() const override;

    void Append(const T& item) override;
    void Prepend(const T& item) override;
    void InsertAt(const T& item, size_t index) override;

    T& operator[](size_t index) override;
    const T& operator[](size_t index) const override;
};

#include "ArraySequence.tpp"
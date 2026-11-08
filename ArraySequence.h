#pragma once
#include <cstddef>
#include <utility>
#include "Sequence.h"
#include "UnqPtr.h"
#include "Exceptions.h"

template <class T>
class ArraySequence : public Sequence<T> {
private:
    UnqPtr<T[]> items;
    size_t count;
    size_t capacity;

public:
    ArraySequence() : items(nullptr), count(0), capacity(0) {}

    ~ArraySequence() override = default; // Память автоматически очищает UnqPtr<T[]>

    ArraySequence(const ArraySequence<T>& other)
        : items(other.capacity > 0 ? new T[other.capacity] : nullptr),
          count(other.count),
          capacity(other.capacity) {
        for (size_t i = 0; i < count; ++i) {
            items[i] = other.items[i];
        }
    }

    ArraySequence<T>& operator=(const ArraySequence<T>& other) {
        if (this != &other) {
            UnqPtr<T[]> newItems(other.capacity > 0 ? new T[other.capacity] : nullptr);
            for (size_t i = 0; i < other.count; ++i) {
                newItems[i] = other.items[i];
            }
            items = std::move(newItems); // Старый буфер удаляется сам внутри UnqPtr
            count = other.count;
            capacity = other.capacity;
        }
        return *this;
    }

    ArraySequence(ArraySequence<T>&& other) noexcept
        : items(std::move(other.items)), count(other.count), capacity(other.capacity) {
        other.count = 0;
        other.capacity = 0;
    }

    ArraySequence<T>& operator=(ArraySequence<T>&& other) noexcept {
        if (this != &other) {
            items = std::move(other.items);
            count = other.count;
            capacity = other.capacity;
            other.count = 0;
            other.capacity = 0;
        }
        return *this;
    }

    const T& Get(size_t index) const override {
        if (index >= count) {
            throw IndexOutOfRange("ArraySequence::Get: Index out of range");
        }
        return items[index];
    }

    T& Get(size_t index) override {
        if (index >= count) {
            throw IndexOutOfRange("ArraySequence::Get: Index out of range");
        }
        return items[index];
    }

    size_t GetLength() const override {
        return count;
    }

    void Append(const T& item) override {
        if (count == capacity) {
            size_t newCapacity = (capacity == 0) ? 2 : capacity * 2;
            UnqPtr<T[]> newItems(new T[newCapacity]);
            for (size_t i = 0; i < count; ++i) {
                newItems[i] = std::move(items[i]);
            }
            items = std::move(newItems); // Старый массив автоматически освобождается UnqPtr
            capacity = newCapacity;
        }
        items[count] = item;
        ++count;
    }

    T& operator[](size_t index) override {
        return Get(index);
    }

    const T& operator[](size_t index) const override {
        return Get(index);
    }
};
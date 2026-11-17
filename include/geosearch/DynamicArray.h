
#pragma once

#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace geosearch {

template <typename T>
class DynamicArray {

private:

    T* data;
    size_t capacity;
    size_t currentSize;

    void resize(size_t newCapacity) {

        T* newData = new T[newCapacity];

        for (size_t i = 0; i < currentSize; ++i) {
            newData[i] = data[i];
        }

        delete[] data;

        data = newData;
        capacity = newCapacity;
    }

public:

    explicit DynamicArray(size_t initialCapacity = 2)
        : capacity(std::max<size_t>(1, initialCapacity)),
          currentSize(0) {

        data = new T[capacity];
    }

    ~DynamicArray() {
        delete[] data;
    }

    DynamicArray(const DynamicArray& other)
        : capacity(other.capacity),
          currentSize(other.currentSize) {

        data = new T[capacity];

        for (size_t i = 0; i < currentSize; ++i) {
            data[i] = other.data[i];
        }
    }

    DynamicArray& operator=(const DynamicArray& other) {

        if (this == &other)
            return *this;

        T* newData = new T[other.capacity];

        for (size_t i = 0; i < other.currentSize; ++i) {
            newData[i] = other.data[i];
        }

        delete[] data;

        data = newData;
        capacity = other.capacity;
        currentSize = other.currentSize;

        return *this;
    }

    void pushBack(const T& value) {

        if (currentSize == capacity) {
            resize(capacity * 2);
        }

        data[currentSize++] = value;
    }

    void insert(size_t index, const T& value) {

        if (index > currentSize)
            throw std::out_of_range("Invalid insertion index");

        if (currentSize == capacity) {
            resize(capacity * 2);
        }

        for (size_t i = currentSize; i > index; --i) {
            data[i] = data[i - 1];
        }

        data[index] = value;
        ++currentSize;
    }

    bool removeAt(size_t index) {

        if (index >= currentSize)
            return false;

        for (size_t i = index; i + 1 < currentSize; ++i) {
            data[i] = data[i + 1];
        }

        --currentSize;

        if (capacity > 2 && currentSize <= capacity / 4) {
            resize(std::max<size_t>(2, capacity / 2));
        }

        return true;
    }

    int search(const T& value) const {

        for (size_t i = 0; i < currentSize; ++i) {

            if (data[i] == value)
                return static_cast<int>(i);
        }

        return -1;
    }

    bool update(size_t index, const T& value) {

        if (index >= currentSize)
            return false;

        data[index] = value;
        return true;
    }

    T& at(size_t index) {

        if (index >= currentSize)
            throw std::out_of_range("Invalid array index");

        return data[index];
    }

    const T& at(size_t index) const {

        if (index >= currentSize)
            throw std::out_of_range("Invalid array index");

        return data[index];
    }

    size_t size() const {
        return currentSize;
    }

    size_t getCapacity() const {
        return capacity;
    }

    bool empty() const {
        return currentSize == 0;
    }

    void display() const {

        for (size_t i = 0; i < currentSize; ++i) {
            std::cout << data[i] << " ";
        }

        std::cout << '\n';
    }
};

} // namespace geosearch

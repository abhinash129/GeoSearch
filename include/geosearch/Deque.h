#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstddef>

namespace geosearch {

template <typename T>
class Deque {

private:

    std::vector<T> data;

    size_t capacity;
    size_t frontIndex = 0;
    size_t rearIndex = 0;
    size_t currentSize = 0;

public:

    explicit Deque(size_t maxCapacity = 5)
        : data(maxCapacity), capacity(maxCapacity) {

        if (capacity == 0) {
            throw std::invalid_argument(
                "Deque capacity must be greater than zero"
            );
        }
    }

    // Insert at front
    bool insertFront(const T& value) {

        if (full())
            return false;

        frontIndex =
            (frontIndex + capacity - 1) % capacity;

        data[frontIndex] = value;

        ++currentSize;

        return true;
    }

    // Insert at rear
    bool insertRear(const T& value) {

        if (full())
            return false;

        data[rearIndex] = value;

        rearIndex = (rearIndex + 1) % capacity;

        ++currentSize;

        return true;
    }

    // Delete from front
    bool deleteFront() {

        if (empty())
            return false;

        frontIndex = (frontIndex + 1) % capacity;

        --currentSize;

        if (empty()) {
            frontIndex = 0;
            rearIndex = 0;
        }

        return true;
    }

    // Delete from rear
    bool deleteRear() {

        if (empty())
            return false;

        rearIndex =
            (rearIndex + capacity - 1) % capacity;

        --currentSize;

        if (empty()) {
            frontIndex = 0;
            rearIndex = 0;
        }

        return true;
    }

    // Get front element
    T& getFront() {

        if (empty())
            throw std::out_of_range("Deque is empty");

        return data[frontIndex];
    }

    const T& getFront() const {

        if (empty())
            throw std::out_of_range("Deque is empty");

        return data[frontIndex];
    }

    // Get rear element
    T& getRear() {

        if (empty())
            throw std::out_of_range("Deque is empty");

        size_t index =
            (rearIndex + capacity - 1) % capacity;

        return data[index];
    }

    const T& getRear() const {

        if (empty())
            throw std::out_of_range("Deque is empty");

        size_t index =
            (rearIndex + capacity - 1) % capacity;

        return data[index];
    }

    // Search element
    int search(const T& value) const {

        for (size_t i = 0; i < currentSize; ++i) {

            size_t index =
                (frontIndex + i) % capacity;

            if (data[index] == value) {
                return static_cast<int>(i);
            }
        }

        return -1;
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

    bool full() const {
        return currentSize == capacity;
    }

    // Display deque
    void display() const {

        std::cout << "FRONT -> ";

        for (size_t i = 0; i < currentSize; ++i) {

            size_t index =
                (frontIndex + i) % capacity;

            std::cout << data[index] << " <-> ";
        }

        std::cout << "REAR\n";
    }
};

} // namespace geosearch
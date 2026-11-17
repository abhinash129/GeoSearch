
#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstddef>

namespace geosearch {

template <typename T>
class CircularQueue {

private:

    std::vector<T> data;

    size_t capacity;
    size_t frontIndex = 0;
    size_t rearIndex = 0;
    size_t currentSize = 0;

public:

    explicit CircularQueue(size_t maxCapacity = 5)
        : data(maxCapacity), capacity(maxCapacity){ 

        if (capacity == 0) {
            throw std::invalid_argument(
                "Circular Queue capacity must be greater than zero"
            );
        }
    }

    bool enqueue(const T& value) {

        if (currentSize == capacity)
            return false;

        data[rearIndex] = value;

        rearIndex = (rearIndex + 1) % capacity;

        ++currentSize;

        return true;
    }

    bool dequeue() {

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

    T& front() {

        if (empty())
            throw std::out_of_range("Circular Queue is empty");

        return data[frontIndex];
    }

    const T& front() const {

        if (empty())
            throw std::out_of_range("Circular Queue is empty");

        return data[frontIndex];
    }

    T& rear() {

        if (empty())
            throw std::out_of_range("Circular Queue is empty");

        size_t index =
            (rearIndex + capacity - 1) % capacity;

        return data[index];
    }

    const T& rear() const {

        if (empty())
            throw std::out_of_range("Circular Queue is empty");

        size_t index =
            (rearIndex + capacity - 1) % capacity;

        return data[index];
    }

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

    void display() const {

        std::cout << "FRONT -> ";

        for (size_t i = 0; i < currentSize; ++i) {

            size_t index =
                (frontIndex + i) % capacity;

            std::cout << data[index] << " -> ";
        }

        std::cout << "REAR\n";
    }
};

} // namespace geosearch

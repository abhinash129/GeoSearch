
#pragma once

#include "Index.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace geosearch {

enum class ProbingStrategy {
    LINEAR,
    QUADRATIC,
    DOUBLE_HASHING
};

class OpenAddressingHashIndex : public Index {

private:

    enum class State {
        EMPTY,
        OCCUPIED,
        DELETED
    };

    struct Entry {
        Place place;
        State state = State::EMPTY;
    };

    Entry* table;
    size_t capacity;
    size_t count;
    ProbingStrategy strategy;

    static constexpr double MAX_LOAD_FACTOR = 0.45;

    static bool isPrime(size_t number) {

        if (number < 2) {
            return false;
        }

        for (size_t i = 2; i <= number / i; ++i) {
            if (number % i == 0) {
                return false;
            }
        }

        return true;
    }

    static size_t nextPrime(size_t number) {

        while (!isPrime(number)) {
            ++number;
        }

        return number;
    }

    size_t hash1(int id) const {
        return static_cast<size_t>(id) % capacity;
    }

    size_t hash2(int id) const {
        return 1 + (static_cast<size_t>(id) % (capacity - 1));
    }

    size_t probeIndex(int id, size_t attempt) const {

        size_t h = hash1(id);

        switch (strategy) {

            case ProbingStrategy::LINEAR:
                return (h + attempt) % capacity;

            case ProbingStrategy::QUADRATIC:
                return (h + attempt * attempt) % capacity;

            case ProbingStrategy::DOUBLE_HASHING:
                return (h + attempt * hash2(id)) % capacity;
        }

        return h;
    }

    // Insert an existing record during resizing.
    void insertWithoutResize(const Place& place) {

        for (size_t i = 0; i < capacity; ++i) {

            size_t index = probeIndex(place.id, i);

            if (table[index].state != State::OCCUPIED) {

                table[index].place = place;
                table[index].state = State::OCCUPIED;

                ++count;
                return;
            }
        }

        throw std::runtime_error("Hash table insertion failed");
    }

    void resize() {

        size_t newCapacity =
            nextPrime(capacity * 2 + 1);

        Entry* oldTable = table;
        size_t oldCapacity = capacity;

        table = new Entry[newCapacity];
        capacity = newCapacity;
        count = 0;

        for (size_t i = 0; i < oldCapacity; ++i) {

            if (oldTable[i].state == State::OCCUPIED) {
                insertWithoutResize(oldTable[i].place);
            }
        }

        delete[] oldTable;
    }

    int findIndex(int id) const {

        if (id <= 0) {
            return -1;
        }

        for (size_t i = 0; i < capacity; ++i) {

            size_t index = probeIndex(id, i);

            if (table[index].state == State::EMPTY) {
                return -1;
            }

            if (table[index].state == State::OCCUPIED &&
                table[index].place.id == id) {

                return static_cast<int>(index);
            }
        }

        return -1;
    }

    std::string strategyName() const {

        switch (strategy) {

            case ProbingStrategy::LINEAR:
                return "Linear Probing";

            case ProbingStrategy::QUADRATIC:
                return "Quadratic Probing";

            case ProbingStrategy::DOUBLE_HASHING:
                return "Double Hashing";
        }

        return "Unknown";
    }

public:

    explicit OpenAddressingHashIndex(
        ProbingStrategy selectedStrategy,
        size_t initialCapacity = 23
    )
        : capacity(nextPrime(initialCapacity)),
          count(0),
          strategy(selectedStrategy) {

        table = new Entry[capacity];
    }

    ~OpenAddressingHashIndex() override {
        delete[] table;
    }

    OpenAddressingHashIndex(
        const OpenAddressingHashIndex&
    ) = delete;

    OpenAddressingHashIndex& operator=(
        const OpenAddressingHashIndex&
    ) = delete;

    bool insert(const Place& place) override {

        if (!place.isValid()) {
            return false;
        }

        if (search(place.id) != nullptr) {
            return false;
        }

        if (static_cast<double>(count + 1) / capacity
            > MAX_LOAD_FACTOR) {

            resize();
        }

        size_t firstDeleted = capacity;

        for (size_t i = 0; i < capacity; ++i) {

            size_t index = probeIndex(place.id, i);

            if (table[index].state == State::OCCUPIED) {
                continue;
            }

            if (table[index].state == State::DELETED) {

                if (firstDeleted == capacity) {
                    firstDeleted = index;
                }

                continue;
            }

            // Empty slot found.
            size_t target = (firstDeleted != capacity)
                                ? firstDeleted
                                : index;

            table[target].place = place;
            table[target].state = State::OCCUPIED;

            ++count;
            return true;
        }

        // Reuse a deleted slot if no empty slot was found.
        if (firstDeleted != capacity) {

            table[firstDeleted].place = place;
            table[firstDeleted].state = State::OCCUPIED;

            ++count;
            return true;
        }

        resize();

        return insert(place);
    }

    Place* search(int id) override {

        int index = findIndex(id);

        if (index == -1) {
            return nullptr;
        }

        return &table[index].place;
    }

    bool remove(int id) override {

        int index = findIndex(id);

        if (index == -1) {
            return false;
        }

        table[index].state = State::DELETED;

        --count;

        return true;
    }

    size_t size() const override {
        return count;
    }

    size_t bucketCount() const {
        return capacity;
    }

    double loadFactor() const {
        return static_cast<double>(count) / capacity;
    }

    std::string name() const override {
        return strategyName();
    }

    void display() const {

        std::cout << "\n" << strategyName() << "\n";

        for (size_t i = 0; i < capacity; ++i) {

            std::cout << "[" << i << "] ";

            if (table[i].state == State::EMPTY) {
                std::cout << "EMPTY";
            }
            else if (table[i].state == State::DELETED) {
                std::cout << "DELETED";
            }
            else {
                std::cout << table[i].place.id
                          << " - "
                          << table[i].place.name;
            }

            std::cout << '\n';
        }

        std::cout << "Records: " << count << '\n';
        std::cout << "Capacity: " << capacity << '\n';
        std::cout << "Load factor: " << loadFactor() << '\n';
    }
};

} // namespace geosearch

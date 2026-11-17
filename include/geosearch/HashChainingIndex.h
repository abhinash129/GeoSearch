
#pragma once

#include "Index.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace geosearch {

class HashChainingIndex : public Index {

private:

    struct Node {
        Place place;
        Node* next;

        explicit Node(const Place& p)
            : place(p), next(nullptr) {}
    };

    Node** buckets;
    size_t capacity;
    size_t count;

    static constexpr double MAX_LOAD_FACTOR = 0.75;

    size_t hashFunction(int id) const {
        return static_cast<size_t>(id) % capacity;
    }

    void resize() {

        size_t newCapacity = capacity * 2 + 1;

        Node** newBuckets = new Node*[newCapacity];

        for (size_t i = 0; i < newCapacity; ++i) {
            newBuckets[i] = nullptr;
        }

        for (size_t i = 0; i < capacity; ++i) {

            Node* current = buckets[i];

            while (current != nullptr) {

                Node* nextNode = current->next;

                size_t newIndex =
                    static_cast<size_t>(current->place.id)
                    % newCapacity;

                current->next = newBuckets[newIndex];
                newBuckets[newIndex] = current;

                current = nextNode;
            }
        }

        delete[] buckets;

        buckets = newBuckets;
        capacity = newCapacity;
    }

public:

    explicit HashChainingIndex(size_t initialCapacity = 11)
        : capacity(initialCapacity), count(0) {

        if (capacity == 0) {
            throw std::invalid_argument(
                "Hash table capacity cannot be zero"
            );
        }

        buckets = new Node*[capacity];

        for (size_t i = 0; i < capacity; ++i) {
            buckets[i] = nullptr;
        }
    }

    ~HashChainingIndex() override {

        for (size_t i = 0; i < capacity; ++i) {

            Node* current = buckets[i];

            while (current != nullptr) {

                Node* nextNode = current->next;

                delete current;

                current = nextNode;
            }
        }

        delete[] buckets;
    }

    HashChainingIndex(const HashChainingIndex&) = delete;
    HashChainingIndex& operator=(const HashChainingIndex&) = delete;

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

        size_t index = hashFunction(place.id);

        Node* newNode = new Node(place);

        newNode->next = buckets[index];
        buckets[index] = newNode;

        ++count;

        return true;
    }

    Place* search(int id) override {

        if (id <= 0) {
            return nullptr;
        }

        size_t index = hashFunction(id);

        Node* current = buckets[index];

        while (current != nullptr) {

            if (current->place.id == id) {
                return &current->place;
            }

            current = current->next;
        }

        return nullptr;
    }

    bool remove(int id) override {

        if (id <= 0) {
            return false;
        }

        size_t index = hashFunction(id);

        Node* current = buckets[index];
        Node* previous = nullptr;

        while (current != nullptr) {

            if (current->place.id == id) {

                if (previous == nullptr) {
                    buckets[index] = current->next;
                } else {
                    previous->next = current->next;
                }

                delete current;

                --count;

                return true;
            }

            previous = current;
            current = current->next;
        }

        return false;
    }

    size_t size() const override {
        return count;
    }

    std::string name() const override {
        return "Hash Table - Separate Chaining";
    }

    double loadFactor() const {
        return static_cast<double>(count) / capacity;
    }

    size_t bucketCount() const {
        return capacity;
    }

    void display() const {

        std::cout << "\nHash Table: Separate Chaining\n";

        for (size_t i = 0; i < capacity; ++i) {

            std::cout << "Bucket " << i << ": ";

            Node* current = buckets[i];

            while (current != nullptr) {

                std::cout << current->place.id;

                if (current->next != nullptr) {
                    std::cout << " -> ";
                }

                current = current->next;
            }

            std::cout << " -> NULL\n";
        }

        std::cout << "Total records: " << count << '\n';
        std::cout << "Capacity: " << capacity << '\n';
        std::cout << "Load factor: " << loadFactor() << '\n';
    }
};

} // namespace geosearch

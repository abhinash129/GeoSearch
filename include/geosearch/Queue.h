
#pragma once

#include <iostream>
#include <cstddef>
#include <stdexcept>

namespace geosearch {

template <typename T>
class Queue {

private:

    struct Node {
        T data;
        Node* next;

        explicit Node(const T& value)
            : data(value), next(nullptr) {}
    };

    Node* frontNode = nullptr;
    Node* rearNode = nullptr;

    size_t currentSize = 0;

public:

    Queue() = default;

    ~Queue() {
        clear();
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void enqueue(const T& value) {

        Node* newNode = new Node(value);

        if (rearNode == nullptr) {
            frontNode = rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }

        ++currentSize;
    }

    bool dequeue() {

        if (empty())
            return false;

        Node* temp = frontNode;

        frontNode = frontNode->next;

        delete temp;

        --currentSize;

        if (frontNode == nullptr) {
            rearNode = nullptr;
        }

        return true;
    }

    T& front() {

        if (empty())
            throw std::out_of_range("Queue is empty");

        return frontNode->data;
    }

    const T& front() const {

        if (empty())
            throw std::out_of_range("Queue is empty");

        return frontNode->data;
    }

    T& rear() {

        if (empty())
            throw std::out_of_range("Queue is empty");

        return rearNode->data;
    }

    const T& rear() const {

        if (empty())
            throw std::out_of_range("Queue is empty");

        return rearNode->data;
    }

    int search(const T& value) const {

        Node* current = frontNode;
        int position = 0;

        while (current != nullptr) {

            if (current->data == value)
                return position;

            current = current->next;
            ++position;
        }

        return -1;
    }

    size_t size() const {
        return currentSize;
    }

    bool empty() const {
        return frontNode == nullptr;
    }

    void display() const {

        Node* current = frontNode;

        std::cout << "FRONT -> ";

        while (current != nullptr) {

            std::cout << current->data << " -> ";

            current = current->next;
        }

        std::cout << "NULL <- REAR\n";
    }

    void clear() {

        while (frontNode != nullptr) {

            Node* temp = frontNode;

            frontNode = frontNode->next;

            delete temp;
        }

        rearNode = nullptr;
        currentSize = 0;
    }
};

} // namespace geosearch

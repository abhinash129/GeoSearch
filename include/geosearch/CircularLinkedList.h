
#pragma once

#include <iostream>
#include <cstddef>

namespace geosearch {

template <typename T>
class CircularLinkedList {

private:

    struct Node {
        T data;
        Node* next;

        explicit Node(const T& value)
            : data(value), next(nullptr) {}
    };

    Node* tail = nullptr;
    size_t currentSize = 0;

public:

    CircularLinkedList() = default;

    ~CircularLinkedList() {
        clear();
    }

    CircularLinkedList(const CircularLinkedList&) = delete;
    CircularLinkedList& operator=(const CircularLinkedList&) = delete;

    void insertFront(const T& value) {

        Node* newNode = new Node(value);

        if (tail == nullptr) {
            tail = newNode;
            newNode->next = newNode;
        } else {
            newNode->next = tail->next;
            tail->next = newNode;
        }

        ++currentSize;
    }

    void insertBack(const T& value) {

        Node* newNode = new Node(value);

        if (tail == nullptr) {
            tail = newNode;
            newNode->next = newNode;
        } else {
            newNode->next = tail->next;
            tail->next = newNode;
            tail = newNode;
        }

        ++currentSize;
    }

    bool insertAt(size_t position, const T& value) {

        if (position > currentSize)
            return false;

        if (position == 0) {
            insertFront(value);
            return true;
        }

        if (position == currentSize) {
            insertBack(value);
            return true;
        }

        Node* current = tail->next;

        for (size_t i = 0; i < position - 1; ++i) {
            current = current->next;
        }

        Node* newNode = new Node(value);

        newNode->next = current->next;
        current->next = newNode;

        ++currentSize;

        return true;
    }

    bool deleteValue(const T& value) {

        if (tail == nullptr)
            return false;

        Node* current = tail->next;
        Node* previous = tail;

        do {

            if (current->data == value) {

                if (current == tail && current == tail->next) {
                    tail = nullptr;
                } else {

                    previous->next = current->next;

                    if (current == tail) {
                        tail = previous;
                    }
                }

                delete current;
                --currentSize;

                return true;
            }

            previous = current;
            current = current->next;

        } while (current != tail->next);

        return false;
    }

    int search(const T& value) const {

        if (tail == nullptr)
            return -1;

        Node* current = tail->next;
        int position = 0;

        do {

            if (current->data == value)
                return position;

            current = current->next;
            ++position;

        } while (current != tail->next);

        return -1;
    }

    bool update(size_t position, const T& value) {

        if (position >= currentSize)
            return false;

        Node* current = tail->next;

        for (size_t i = 0; i < position; ++i) {
            current = current->next;
        }

        current->data = value;

        return true;
    }

    void display() const {

        if (tail == nullptr) {
            std::cout << "EMPTY\n";
            return;
        }

        Node* current = tail->next;

        do {

            std::cout << current->data << " -> ";

            current = current->next;

        } while (current != tail->next);

        std::cout << "(HEAD)\n";
    }

    size_t size() const {
        return currentSize;
    }

    bool empty() const {
        return tail == nullptr;
    }

    void clear() {

        if (tail == nullptr)
            return;

        Node* head = tail->next;
        tail->next = nullptr;

        while (head != nullptr) {

            Node* temp = head;
            head = head->next;

            delete temp;
        }

        tail = nullptr;
        currentSize = 0;
    }
};

} // namespace geosearch

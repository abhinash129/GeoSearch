
#pragma once

#include <iostream>
#include <cstddef>

namespace geosearch {

template <typename T>
class DoublyLinkedList {

private:

    struct Node {
        T data;
        Node* previous;
        Node* next;

        explicit Node(const T& value)
            : data(value), previous(nullptr), next(nullptr) {}
    };

    Node* head = nullptr;
    Node* tail = nullptr;

    size_t currentSize = 0;

public:

    DoublyLinkedList() = default;

    ~DoublyLinkedList() {
        clear();
    }

    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    void insertFront(const T& value) {

        Node* newNode = new Node(value);

        if (head == nullptr) {
            head = tail = newNode;
        } else {

            newNode->next = head;
            head->previous = newNode;
            head = newNode;
        }

        ++currentSize;
    }

    void insertBack(const T& value) {

        Node* newNode = new Node(value);

        if (tail == nullptr) {
            head = tail = newNode;
        } else {

            newNode->previous = tail;
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

        Node* current;

        if (position < currentSize / 2) {

            current = head;

            for (size_t i = 0; i < position; ++i) {
                current = current->next;
            }

        } else {

            current = tail;

            for (size_t i = currentSize - 1; i > position; --i) {
                current = current->previous;
            }
        }

        Node* newNode = new Node(value);

        newNode->previous = current->previous;
        newNode->next = current;

        current->previous->next = newNode;
        current->previous = newNode;

        ++currentSize;

        return true;
    }

    bool deleteValue(const T& value) {

        Node* current = head;

        while (current != nullptr &&
               !(current->data == value)) {

            current = current->next;
        }

        if (current == nullptr)
            return false;

        if (current->previous != nullptr) {
            current->previous->next = current->next;
        } else {
            head = current->next;
        }

        if (current->next != nullptr) {
            current->next->previous = current->previous;
        } else {
            tail = current->previous;
        }

        delete current;

        --currentSize;

        return true;
    }

    int search(const T& value) const {

        Node* current = head;
        int position = 0;

        while (current != nullptr) {

            if (current->data == value)
                return position;

            current = current->next;
            ++position;
        }

        return -1;
    }

    bool update(size_t position, const T& value) {

        if (position >= currentSize)
            return false;

        Node* current;

        if (position < currentSize / 2) {

            current = head;

            for (size_t i = 0; i < position; ++i) {
                current = current->next;
            }

        } else {

            current = tail;

            for (size_t i = currentSize - 1; i > position; --i) {
                current = current->previous;
            }
        }

        current->data = value;

        return true;
    }

    void displayForward() const {

        Node* current = head;

        while (current != nullptr) {

            std::cout << current->data << " <-> ";

            current = current->next;
        }

        std::cout << "NULL\n";
    }

    void displayBackward() const {

        Node* current = tail;

        while (current != nullptr) {

            std::cout << current->data << " <-> ";

            current = current->previous;
        }

        std::cout << "NULL\n";
    }

    size_t size() const {
        return currentSize;
    }

    bool empty() const {
        return currentSize == 0;
    }

    void clear() {

        Node* current = head;

        while (current != nullptr) {

            Node* temp = current;

            current = current->next;

            delete temp;
        }

        head = nullptr;
        tail = nullptr;
        currentSize = 0;
    }
};

} // namespace geosearch

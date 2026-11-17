
#pragma once

#include <iostream>
#include <cstddef>

namespace geosearch {

template <typename T>
class SinglyLinkedList {

private:

    struct Node {
        T data;
        Node* next;

        explicit Node(const T& value)
            : data(value), next(nullptr) {}
    };

    Node* head = nullptr;
    size_t currentSize = 0;

public:

    SinglyLinkedList() = default;

    ~SinglyLinkedList() {
        clear();
    }

    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    void insertFront(const T& value) {

        Node* newNode = new Node(value);

        newNode->next = head;
        head = newNode;

        ++currentSize;
    }

    void insertBack(const T& value) {

        Node* newNode = new Node(value);

        if (head == nullptr) {
            head = newNode;
        } else {

            Node* current = head;

            while (current->next != nullptr) {
                current = current->next;
            }

            current->next = newNode;
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

        Node* current = head;

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

        if (head == nullptr)
            return false;

        if (head->data == value) {

            Node* temp = head;
            head = head->next;

            delete temp;
            --currentSize;

            return true;
        }

        Node* current = head;

        while (current->next != nullptr &&
               !(current->next->data == value)) {

            current = current->next;
        }

        if (current->next == nullptr)
            return false;

        Node* temp = current->next;

        current->next = temp->next;

        delete temp;
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

        Node* current = head;

        for (size_t i = 0; i < position; ++i) {
            current = current->next;
        }

        current->data = value;

        return true;
    }

    void display() const {

        Node* current = head;

        while (current != nullptr) {

            std::cout << current->data << " -> ";

            current = current->next;
        }

        std::cout << "NULL\n";
    }

    size_t size() const {
        return currentSize;
    }

    bool empty() const {
        return head == nullptr;
    }

    void clear() {

        while (head != nullptr) {

            Node* temp = head;

            head = head->next;

            delete temp;
        }

        currentSize = 0;
    }
};

} // namespace geosearch

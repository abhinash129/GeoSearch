
#pragma once

#include "Index.h"

#include <string>

namespace geosearch {

class BSTIndex : public Index {

private:

    struct Node {
        Place place;
        Node* left;
        Node* right;

        explicit Node(const Place& p)
            : place(p), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;
    size_t count = 0;

    Node* insertNode(
        Node* node,
        const Place& place,
        bool& inserted
    ) {

        if (node == nullptr) {
            inserted = true;
            return new Node(place);
        }

        if (place.id < node->place.id) {
            node->left = insertNode(node->left, place, inserted);
        }
        else if (place.id > node->place.id) {
            node->right = insertNode(node->right, place, inserted);
        }

        return node;
    }

    Node* findMinimum(Node* node) const {

        while (node != nullptr && node->left != nullptr) {
            node = node->left;
        }

        return node;
    }

    Node* removeNode(
        Node* node,
        int id,
        bool& removed
    ) {

        if (node == nullptr) {
            return nullptr;
        }

        if (id < node->place.id) {
            node->left = removeNode(node->left, id, removed);
        }
        else if (id > node->place.id) {
            node->right = removeNode(node->right, id, removed);
        }
        else {

            removed = true;

            if (node->left == nullptr) {
                Node* temp = node->right;
                delete node;
                return temp;
            }

            if (node->right == nullptr) {
                Node* temp = node->left;
                delete node;
                return temp;
            }

            Node* successor = findMinimum(node->right);

            node->place = successor->place;

            bool successorRemoved = false;

            node->right = removeNode(
                node->right,
                successor->place.id,
                successorRemoved
            );
        }

        return node;
    }

    Node* searchNode(Node* node, int id) const {

        if (node == nullptr || node->place.id == id) {
            return node;
        }

        if (id < node->place.id) {
            return searchNode(node->left, id);
        }

        return searchNode(node->right, id);
    }

    void destroy(Node* node) {

        if (node == nullptr) {
            return;
        }

        destroy(node->left);
        destroy(node->right);

        delete node;
    }

public:

    BSTIndex() = default;

    ~BSTIndex() override {
        destroy(root);
    }

    BSTIndex(const BSTIndex&) = delete;
    BSTIndex& operator=(const BSTIndex&) = delete;

    bool insert(const Place& place) override {

        if (!place.isValid()) {
            return false;
        }

        bool inserted = false;

        root = insertNode(root, place, inserted);

        if (inserted) {
            ++count;
        }

        return inserted;
    }

    Place* search(int id) override {

        if (id <= 0) {
            return nullptr;
        }

        Node* result = searchNode(root, id);

        return result != nullptr ? &result->place : nullptr;
    }

    bool remove(int id) override {

        bool removed = false;

        root = removeNode(root, id, removed);

        if (removed) {
            --count;
        }

        return removed;
    }

    size_t size() const override {
        return count;
    }

    std::string name() const override {
        return "Binary Search Tree";
    }
};

} // namespace geosearch

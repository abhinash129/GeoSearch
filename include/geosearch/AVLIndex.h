
#pragma once

#include "Index.h"

#include <algorithm>
#include <string>

namespace geosearch {

class AVLIndex : public Index {

private:

    struct Node {
        Place place;
        Node* left;
        Node* right;
        int height;

        explicit Node(const Place& p)
            : place(p),
              left(nullptr),
              right(nullptr),
              height(1) {}
    };

    Node* root = nullptr;
    size_t count = 0;

    int height(Node* node) const {
        return node == nullptr ? 0 : node->height;
    }

    int balanceFactor(Node* node) const {
        return node == nullptr
                   ? 0
                   : height(node->left) - height(node->right);
    }

    void updateHeight(Node* node) {

        node->height = 1 + std::max(
            height(node->left),
            height(node->right)
        );
    }

    Node* rotateRight(Node* y) {

        Node* x = y->left;
        Node* temp = x->right;

        x->right = y;
        y->left = temp;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    Node* rotateLeft(Node* x) {

        Node* y = x->right;
        Node* temp = y->left;

        y->left = x;
        x->right = temp;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    Node* balance(Node* node) {

        if (node == nullptr) {
            return nullptr;
        }

        updateHeight(node);

        int bf = balanceFactor(node);

        // Left Left case
        if (bf > 1 && balanceFactor(node->left) >= 0) {
            return rotateRight(node);
        }

        // Left Right case
        if (bf > 1 && balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        // Right Right case
        if (bf < -1 && balanceFactor(node->right) <= 0) {
            return rotateLeft(node);
        }

        // Right Left case
        if (bf < -1 && balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

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
        else {
            return node;
        }

        return balance(node);
    }

    Node* findMinimum(Node* node) const {

        while (node->left != nullptr) {
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

            if (node->left == nullptr ||
                node->right == nullptr) {

                Node* child = node->left != nullptr
                                  ? node->left
                                  : node->right;

                delete node;

                return child;
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

        return balance(node);
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

    AVLIndex() = default;

    ~AVLIndex() override {
        destroy(root);
    }

    AVLIndex(const AVLIndex&) = delete;
    AVLIndex& operator=(const AVLIndex&) = delete;

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
        return "AVL Tree";
    }
};

} // namespace geosearch

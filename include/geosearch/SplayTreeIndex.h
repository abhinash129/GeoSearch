
#pragma once

#include "Index.h"

#include <string>
#include <vector>

namespace geosearch {

class SplayTreeIndex : public Index {

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

    Node* rotateRight(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        y->right = x;
        return y;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        return y;
    }

    Node* splay(Node* node, int id) {

        if (node == nullptr || node->place.id == id)
            return node;

        if (id < node->place.id) {

            if (node->left == nullptr)
                return node;

            if (id < node->left->place.id) {
                node->left->left =
                    splay(node->left->left, id);

                node = rotateRight(node);
            }
            else if (id > node->left->place.id) {
                node->left->right =
                    splay(node->left->right, id);

                if (node->left->right != nullptr)
                    node->left = rotateLeft(node->left);
            }

            return node->left == nullptr
                       ? node
                       : rotateRight(node);
        }

        else {

            if (node->right == nullptr)
                return node;

            if (id > node->right->place.id) {
                node->right->right =
                    splay(node->right->right, id);

                node = rotateLeft(node);
            }
            else if (id < node->right->place.id) {
                node->right->left =
                    splay(node->right->left, id);

                if (node->right->left != nullptr)
                    node->right = rotateRight(node->right);
            }

            return node->right == nullptr
                       ? node
                       : rotateLeft(node);
        }
    }

    void destroy(Node* node) {
        if (node == nullptr) return;

        std::vector<Node*> pending;
        pending.push_back(node);

        while (!pending.empty()) {
            Node* current = pending.back();
            pending.pop_back();

            if (current->left != nullptr)
                pending.push_back(current->left);

            if (current->right != nullptr)
                pending.push_back(current->right);

            delete current;
        }
    }

public:

    SplayTreeIndex() = default;

    ~SplayTreeIndex() override {
        destroy(root);
    }

    SplayTreeIndex(const SplayTreeIndex&) = delete;
    SplayTreeIndex& operator=(const SplayTreeIndex&) = delete;

    bool insert(const Place& place) override {

        if (!place.isValid())
            return false;

        if (root == nullptr) {
            root = new Node(place);
            ++count;
            return true;
        }

        root = splay(root, place.id);

        if (root->place.id == place.id)
            return false;

        Node* newNode = new Node(place);

        if (place.id < root->place.id) {

            newNode->right = root;
            newNode->left = root->left;
            root->left = nullptr;

        } else {

            newNode->left = root;
            newNode->right = root->right;
            root->right = nullptr;
        }

        root = newNode;
        ++count;

        return true;
    }

    Place* search(int id) override {

        if (id <= 0 || root == nullptr)
            return nullptr;

        root = splay(root, id);

        if (root->place.id == id)
            return &root->place;

        return nullptr;
    }

    bool remove(int id) override {

        if (id <= 0 || root == nullptr)
            return false;

        root = splay(root, id);

        if (root->place.id != id)
            return false;

        Node* oldRoot = root;

        if (root->left == nullptr) {

            root = root->right;

        } else {

            Node* rightSubtree = root->right;

            root = root->left;

            root = splay(root, id);

            root->right = rightSubtree;
        }

        delete oldRoot;
        --count;

        return true;
    }

    size_t size() const override {
        return count;
    }

    std::string name() const override {
        return "Splay Tree";
    }
};

} // namespace geosearch

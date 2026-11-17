
#pragma once

#include "Place.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

namespace geosearch {

class FibonacciHeap {

private:

    struct Node {
        Place place;
        double priority;

        int degree = 0;
        bool marked = false;

        Node* parent = nullptr;
        Node* child = nullptr;

        Node* left = this;
        Node* right = this;

        Node(const Place& p, double value)
            : place(p), priority(value) {}
    };

    Node* minimum = nullptr;
    size_t count = 0;

    std::unordered_map<int, Node*> nodes;

    void insertAfter(Node* position, Node* node) {

        node->left = position;
        node->right = position->right;

        position->right->left = node;
        position->right = node;
    }

    void removeFromList(Node* node) {

        node->left->right = node->right;
        node->right->left = node->left;

        node->left = node;
        node->right = node;
    }

    void addToRootList(Node* node) {

        node->parent = nullptr;
        node->marked = false;

        if (minimum == nullptr) {

            minimum = node;
            node->left = node;
            node->right = node;

        } else {

            insertAfter(minimum, node);

            if (node->priority < minimum->priority)
                minimum = node;
        }
    }

    void link(Node* child, Node* parent) {

        removeFromList(child);

        child->parent = parent;
        child->marked = false;

        if (parent->child == nullptr) {

            parent->child = child;
            child->left = child;
            child->right = child;

        } else {

            insertAfter(parent->child, child);
        }

        ++parent->degree;
    }

    void consolidate() {

        if (minimum == nullptr)
            return;

        std::vector<Node*> roots;

        Node* current = minimum;

        do {
            roots.push_back(current);
            current = current->right;
        } while (current != minimum);

        std::vector<Node*> degreeTable(count + 2, nullptr);

        for (Node* node : roots) {

            int degree = node->degree;

            while (degreeTable[degree] != nullptr) {

                Node* other = degreeTable[degree];

                if (other->priority < node->priority)
                    std::swap(node, other);

                degreeTable[degree] = nullptr;

                link(other, node);

                ++degree;
            }

            degreeTable[degree] = node;
        }

        minimum = nullptr;

        for (Node* node : degreeTable) {

            if (node == nullptr)
                continue;

            node->left = node;
            node->right = node;

            if (minimum == nullptr) {

                minimum = node;

            } else {

                insertAfter(minimum, node);

                if (node->priority < minimum->priority)
                    minimum = node;
            }
        }
    }

    void cut(Node* node, Node* parent) {

        if (parent->child == node) {

            if (node->right == node)
                parent->child = nullptr;
            else
                parent->child = node->right;
        }

        removeFromList(node);

        --parent->degree;

        addToRootList(node);
    }

    void cascadingCut(Node* node) {

        Node* parent = node->parent;

        if (parent == nullptr)
            return;

        if (!node->marked) {

            node->marked = true;

        } else {

            cut(node, parent);
            cascadingCut(parent);
        }
    }

    void removeMinimum() {

        if (minimum == nullptr)
            return;

        Node* oldMinimum = minimum;

        if (oldMinimum->child != nullptr) {

            std::vector<Node*> children;

            Node* child = oldMinimum->child;

            do {
                children.push_back(child);
                child = child->right;
            } while (child != oldMinimum->child);

            for (Node* node : children) {

                removeFromList(node);

                node->parent = nullptr;
                node->marked = false;

                insertAfter(oldMinimum, node);
            }

            oldMinimum->child = nullptr;
        }

        Node* nextRoot = oldMinimum->right;

        removeFromList(oldMinimum);

        if (nextRoot == oldMinimum) {

            minimum = nullptr;

        } else {

            minimum = nextRoot;
            consolidate();
        }

        nodes.erase(oldMinimum->place.id);

        delete oldMinimum;
        --count;
    }

    void destroyList(Node* start) {

        if (start == nullptr)
            return;

        std::vector<Node*> pending;
        std::vector<Node*> visited;

        pending.push_back(start);

        while (!pending.empty()) {

            Node* current = pending.back();
            pending.pop_back();

            visited.push_back(current);

            if (current->child != nullptr) {

                Node* child = current->child;

                do {
                    pending.push_back(child);
                    child = child->right;
                } while (child != current->child);
            }
        }

        for (Node* node : visited)
            delete node;
    }

public:

    FibonacciHeap() = default;

    ~FibonacciHeap() {
        destroyList(minimum);
    }

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    bool insert(const Place& place, double priority) {

        if (!place.isValid() ||
            !std::isfinite(priority) ||
            nodes.find(place.id) != nodes.end()) {
            return false;
        }

        Node* node = new Node(place, priority);

        addToRootList(node);

        nodes[place.id] = node;
        ++count;

        return true;
    }

    const Place* peek() const {

        return minimum != nullptr
                   ? &minimum->place
                   : nullptr;
    }

    bool extractMin(Place& result) {

        if (minimum == nullptr)
            return false;

        result = minimum->place;

        removeMinimum();

        return true;
    }

    const Place* search(int id) const {

        auto it = nodes.find(id);

        return it == nodes.end()
                   ? nullptr
                   : &it->second->place;
    }

    bool decreaseKey(int id, double newPriority) {

        auto it = nodes.find(id);

        if (it == nodes.end() ||
            !std::isfinite(newPriority)) {
            return false;
        }

        Node* node = it->second;

        if (newPriority > node->priority)
            return false;

        node->priority = newPriority;

        Node* parent = node->parent;

        if (parent != nullptr &&
            node->priority < parent->priority) {

            cut(node, parent);
            cascadingCut(parent);
        }

        if (minimum == nullptr ||
            node->priority < minimum->priority) {
            minimum = node;
        }

        return true;
    }

    bool remove(int id) {

        if (!decreaseKey(
                id,
                -std::numeric_limits<double>::max())) {
            return false;
        }

        Place removed;
        return extractMin(removed);
    }

    size_t size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }
};

} // namespace geosearch

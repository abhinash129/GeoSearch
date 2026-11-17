
#pragma once

#include "Place.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace geosearch {

class BinomialHeap {

private:

    struct Node {
        Place place;
        double priority;
        int degree;

        Node* parent;
        Node* child;
        Node* sibling;

        Node(const Place& p, double value)
            : place(p),
              priority(value),
              degree(0),
              parent(nullptr),
              child(nullptr),
              sibling(nullptr) {}
    };

    Node* head = nullptr;

    void linkTrees(Node* child, Node* parent) {

        child->parent = parent;
        child->sibling = parent->child;
        parent->child = child;

        ++parent->degree;
    }

    Node* mergeRootLists(Node* first, Node* second) {

        if (first == nullptr) return second;
        if (second == nullptr) return first;

        Node* result = nullptr;
        Node** tail = &result;

        while (first != nullptr && second != nullptr) {

            if (first->degree <= second->degree) {
                *tail = first;
                first = first->sibling;
            } else {
                *tail = second;
                second = second->sibling;
            }

            tail = &((*tail)->sibling);
        }

        *tail = first != nullptr ? first : second;

        return result;
    }

    void consolidate() {

        if (head == nullptr)
            return;

        Node* previous = nullptr;
        Node* current = head;
        Node* next = current->sibling;

        while (next != nullptr) {

            if (current->degree != next->degree ||
                (next->sibling != nullptr &&
                 next->sibling->degree == current->degree)) {

                previous = current;
                current = next;

            } else if (current->priority <= next->priority) {

                current->sibling = next->sibling;
                linkTrees(next, current);

            } else {

                if (previous == nullptr) {
                    head = next;
                } else {
                    previous->sibling = next;
                }

                linkTrees(current, next);

                current = next;
            }

            next = current->sibling;
        }
    }

    void unite(Node* other) {

        head = mergeRootLists(head, other);
        consolidate();
    }

    Node* findNode(Node* node, int id) const {

        while (node != nullptr) {

            if (node->place.id == id)
                return node;

            Node* found = findNode(node->child, id);

            if (found != nullptr)
                return found;

            node = node->sibling;
        }

        return nullptr;
    }

    Node* findMinimumNode() const {

        Node* minimum = nullptr;

        for (Node* current = head;
             current != nullptr;
             current = current->sibling) {

            if (minimum == nullptr ||
                current->priority < minimum->priority) {
                minimum = current;
            }
        }

        return minimum;
    }

    void destroy(Node* node) {

        while (node != nullptr) {

            Node* next = node->sibling;

            destroy(node->child);

            delete node;
            node = next;
        }
    }

    bool decreaseKey(Node* node, double newPriority) {

        if (node == nullptr ||
            !std::isfinite(newPriority) ||
            newPriority > node->priority) {
            return false;
        }

        node->priority = newPriority;

        while (node->parent != nullptr &&
               node->priority < node->parent->priority) {

            std::swap(node->place, node->parent->place);
            std::swap(node->priority, node->parent->priority);

            node = node->parent;
        }

        return true;
    }

public:

    BinomialHeap() = default;

    ~BinomialHeap() {
        destroy(head);
    }

    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;

    bool insert(const Place& place, double priority) {

        if (!place.isValid() ||
            !std::isfinite(priority) ||
            search(place.id) != nullptr) {
            return false;
        }

        Node* node = new Node(place, priority);

        unite(node);

        return true;
    }

    const Place* peek() const {

        Node* minimum = findMinimumNode();

        return minimum != nullptr
                   ? &minimum->place
                   : nullptr;
    }

    bool extractMin(Place& result) {

        if (head == nullptr)
            return false;

        Node* minimum = head;
        Node* previousMinimum = nullptr;

        Node* previous = nullptr;

        for (Node* current = head;
             current != nullptr;
             current = current->sibling) {

            if (current->priority < minimum->priority) {
                minimum = current;
                previousMinimum = previous;
            }

            previous = current;
        }

        result = minimum->place;

        if (previousMinimum == nullptr) {
            head = minimum->sibling;
        } else {
            previousMinimum->sibling = minimum->sibling;
        }

        Node* reversedChildren = nullptr;
        Node* child = minimum->child;

        while (child != nullptr) {

            Node* next = child->sibling;

            child->parent = nullptr;
            child->sibling = reversedChildren;

            reversedChildren = child;
            child = next;
        }

        minimum->child = nullptr;
        delete minimum;

        unite(reversedChildren);

        return true;
    }

    const Place* search(int id) const {

        Node* result = findNode(head, id);

        return result != nullptr
                   ? &result->place
                   : nullptr;
    }

    bool decreaseKey(int id, double newPriority) {

        return decreaseKey(findNode(head, id), newPriority);
    }

    bool remove(int id) {

        Node* node = findNode(head, id);

        if (node == nullptr)
            return false;

        decreaseKey(
            node,
            -std::numeric_limits<double>::max()
        );

        Place removed;
        return extractMin(removed);
    }

    size_t size() const {

        size_t total = 0;
        std::vector<Node*> pending;

        for (Node* node = head;
             node != nullptr;
             node = node->sibling) {
            pending.push_back(node);
        }

        while (!pending.empty()) {

            Node* current = pending.back();
            pending.pop_back();

            ++total;

            for (Node* child = current->child;
                 child != nullptr;
                 child = child->sibling) {
                pending.push_back(child);
            }
        }

        return total;
    }

    bool empty() const {
        return head == nullptr;
    }
};

} // namespace geosearch


#pragma once

#include "Index.h"

#include <algorithm>
#include <string>
#include <vector>

namespace geosearch {

class BTreeIndex : public Index {

private:

    static constexpr size_t T = 3;

    struct Node {
        bool leaf;
        std::vector<Place> keys;
        std::vector<Node*> children;

        explicit Node(bool isLeaf)
            : leaf(isLeaf) {}
    };

    Node* root = nullptr;
    size_t count = 0;

    size_t findKey(Node* node, int id) const {
        size_t index = 0;

        while (index < node->keys.size() &&
               node->keys[index].id < id) {
            ++index;
        }

        return index;
    }

    Place* searchNode(Node* node, int id) const {

        if (node == nullptr)
            return nullptr;

        size_t index = findKey(node, id);

        if (index < node->keys.size() &&
            node->keys[index].id == id) {
            return &node->keys[index];
        }

        if (node->leaf)
            return nullptr;

        return searchNode(node->children[index], id);
    }

    void splitChild(Node* parent, size_t index) {

        Node* fullChild = parent->children[index];
        Node* newChild = new Node(fullChild->leaf);

        Place middleKey = fullChild->keys[T - 1];

        for (size_t i = T; i < fullChild->keys.size(); ++i)
            newChild->keys.push_back(fullChild->keys[i]);

        fullChild->keys.resize(T - 1);

        if (!fullChild->leaf) {

            for (size_t i = T;
                 i < fullChild->children.size(); ++i) {
                newChild->children.push_back(
                    fullChild->children[i]
                );
            }

            fullChild->children.resize(T);
        }

        parent->children.insert(
            parent->children.begin() + index + 1,
            newChild
        );

        parent->keys.insert(
            parent->keys.begin() + index,
            middleKey
        );
    }

    void insertNonFull(Node* node, const Place& place) {

        size_t index = node->keys.size();

        if (node->leaf) {

            auto position = std::lower_bound(
                node->keys.begin(),
                node->keys.end(),
                place.id,
                [](const Place& p, int id) {
                    return p.id < id;
                }
            );

            node->keys.insert(position, place);
            return;
        }

        while (index > 0 &&
               place.id < node->keys[index - 1].id) {
            --index;
        }

        if (node->children[index]->keys.size() == 2 * T - 1) {

            splitChild(node, index);

            if (place.id > node->keys[index].id)
                ++index;
        }

        insertNonFull(node->children[index], place);
    }

    Place getPredecessor(Node* node) const {

        while (!node->leaf)
            node = node->children.back();

        return node->keys.back();
    }

    Place getSuccessor(Node* node) const {

        while (!node->leaf)
            node = node->children.front();

        return node->keys.front();
    }

    void borrowFromPrevious(Node* node, size_t index) {

        Node* child = node->children[index];
        Node* sibling = node->children[index - 1];

        child->keys.insert(
            child->keys.begin(),
            node->keys[index - 1]
        );

        if (!child->leaf) {

            child->children.insert(
                child->children.begin(),
                sibling->children.back()
            );

            sibling->children.pop_back();
        }

        node->keys[index - 1] = sibling->keys.back();
        sibling->keys.pop_back();
    }

    void borrowFromNext(Node* node, size_t index) {

        Node* child = node->children[index];
        Node* sibling = node->children[index + 1];

        child->keys.push_back(node->keys[index]);

        if (!child->leaf) {

            child->children.push_back(
                sibling->children.front()
            );

            sibling->children.erase(
                sibling->children.begin()
            );
        }

        node->keys[index] = sibling->keys.front();

        sibling->keys.erase(
            sibling->keys.begin()
        );
    }

    void merge(Node* node, size_t index) {

        Node* child = node->children[index];
        Node* sibling = node->children[index + 1];

        child->keys.push_back(node->keys[index]);

        for (const Place& place : sibling->keys)
            child->keys.push_back(place);

        if (!child->leaf) {

            for (Node* nextChild : sibling->children)
                child->children.push_back(nextChild);
        }

        node->keys.erase(
            node->keys.begin() + index
        );

        node->children.erase(
            node->children.begin() + index + 1
        );

        delete sibling;
    }

    void fill(Node* node, size_t index) {

        if (index > 0 &&
            node->children[index - 1]->keys.size() >= T) {

            borrowFromPrevious(node, index);

        }
        else if (index < node->keys.size() &&
                 node->children[index + 1]->keys.size() >= T) {

            borrowFromNext(node, index);

        }
        else {

            if (index < node->keys.size())
                merge(node, index);
            else
                merge(node, index - 1);
        }
    }

    void removeFromInternal(
        Node* node,
        size_t index
    ) {

        int id = node->keys[index].id;

        Node* leftChild = node->children[index];
        Node* rightChild = node->children[index + 1];

        if (leftChild->keys.size() >= T) {

            Place predecessor = getPredecessor(leftChild);

            node->keys[index] = predecessor;

            removeFromNode(leftChild, predecessor.id);
        }
        else if (rightChild->keys.size() >= T) {

            Place successor = getSuccessor(rightChild);

            node->keys[index] = successor;

            removeFromNode(rightChild, successor.id);
        }
        else {

            merge(node, index);

            removeFromNode(node->children[index], id);
        }
    }

    void removeFromNode(Node* node, int id) {

        size_t index = findKey(node, id);

        if (index < node->keys.size() &&
            node->keys[index].id == id) {

            if (node->leaf) {

                node->keys.erase(
                    node->keys.begin() + index
                );

            } else {

                removeFromInternal(node, index);
            }

            return;
        }

        if (node->leaf)
            return;

        bool wasLastChild = (index == node->keys.size());

        if (node->children[index]->keys.size() < T)
            fill(node, index);

        if (wasLastChild && index > node->keys.size())
            removeFromNode(node->children[index - 1], id);
        else
            removeFromNode(node->children[index], id);
    }

    void destroy(Node* node) {

        if (node == nullptr)
            return;

        for (Node* child : node->children)
            destroy(child);

        delete node;
    }

public:

    BTreeIndex() = default;

    ~BTreeIndex() override {
        destroy(root);
    }

    BTreeIndex(const BTreeIndex&) = delete;
    BTreeIndex& operator=(const BTreeIndex&) = delete;

    bool insert(const Place& place) override {

        if (!place.isValid())
            return false;

        if (search(place.id) != nullptr)
            return false;

        if (root == nullptr) {

            root = new Node(true);
            root->keys.push_back(place);

        } else {

            if (root->keys.size() == 2 * T - 1) {

                Node* newRoot = new Node(false);

                newRoot->children.push_back(root);

                splitChild(newRoot, 0);

                root = newRoot;
            }

            insertNonFull(root, place);
        }

        ++count;
        return true;
    }

    Place* search(int id) override {

        if (id <= 0)
            return nullptr;

        return searchNode(root, id);
    }

    bool remove(int id) override {

        if (id <= 0 || root == nullptr)
            return false;

        if (search(id) == nullptr)
            return false;

        removeFromNode(root, id);

        if (root->keys.empty()) {

            Node* oldRoot = root;

            if (root->leaf)
                root = nullptr;
            else
                root = root->children[0];

            delete oldRoot;
        }

        --count;
        return true;
    }

    size_t size() const override {
        return count;
    }

    std::string name() const override {
        return "B-Tree";
    }
};

} // namespace geosearch

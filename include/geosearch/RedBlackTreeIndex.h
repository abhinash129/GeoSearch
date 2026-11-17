#pragma once

#include "Index.h"

#include <string>

namespace geosearch {

class RedBlackTreeIndex : public Index {

private:

    enum Color {
        RED,
        BLACK
    };

    struct Node {
        Place place;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node()
            : color(BLACK),
              left(nullptr),
              right(nullptr),
              parent(nullptr) {}

        explicit Node(const Place& p)
            : place(p),
              color(RED),
              left(nullptr),
              right(nullptr),
              parent(nullptr) {}
    };

    Node* NIL;
    Node* root;
    size_t count = 0;

    void leftRotate(Node* x) {
        Node* y = x->right;

        x->right = y->left;

        if (y->left != NIL) {
            y->left->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == NIL) {
            root = y;
        }
        else if (x == x->parent->left) {
            x->parent->left = y;
        }
        else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    void rightRotate(Node* y) {
        Node* x = y->left;

        y->left = x->right;

        if (x->right != NIL) {
            x->right->parent = y;
        }

        x->parent = y->parent;

        if (y->parent == NIL) {
            root = x;
        }
        else if (y == y->parent->right) {
            y->parent->right = x;
        }
        else {
            y->parent->left = x;
        }

        x->right = y;
        y->parent = x;
    }

    void insertFix(Node* z) {
        while (z->parent->color == RED) {

            if (z->parent == z->parent->parent->left) {

                Node* uncle = z->parent->parent->right;

                if (uncle->color == RED) {

                    z->parent->color = BLACK;
                    uncle->color = BLACK;
                    z->parent->parent->color = RED;

                    z = z->parent->parent;
                }
                else {

                    if (z == z->parent->right) {
                        z = z->parent;
                        leftRotate(z);
                    }

                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;

                    rightRotate(z->parent->parent);
                }
            }
            else {

                Node* uncle = z->parent->parent->left;

                if (uncle->color == RED) {

                    z->parent->color = BLACK;
                    uncle->color = BLACK;
                    z->parent->parent->color = RED;

                    z = z->parent->parent;
                }
                else {

                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }

                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;

                    leftRotate(z->parent->parent);
                }
            }
        }

        root->color = BLACK;
    }

    Node* searchNode(int id) const {
        Node* current = root;

        while (current != NIL && current->place.id != id) {

            if (id < current->place.id) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }

        return current;
    }

    Node* minimum(Node* node) const {
        while (node->left != NIL) {
            node = node->left;
        }

        return node;
    }

    void transplant(Node* u, Node* v) {

        if (u->parent == NIL) {
            root = v;
        }
        else if (u == u->parent->left) {
            u->parent->left = v;
        }
        else {
            u->parent->right = v;
        }

        v->parent = u->parent;
    }

    void deleteFix(Node* x) {

        while (x != root && x->color == BLACK) {

            if (x == x->parent->left) {

                Node* sibling = x->parent->right;

                if (sibling->color == RED) {

                    sibling->color = BLACK;
                    x->parent->color = RED;

                    leftRotate(x->parent);

                    sibling = x->parent->right;
                }

                if (sibling->left->color == BLACK &&
                    sibling->right->color == BLACK) {

                    sibling->color = RED;
                    x = x->parent;
                }
                else {

                    if (sibling->right->color == BLACK) {

                        sibling->left->color = BLACK;
                        sibling->color = RED;

                        rightRotate(sibling);

                        sibling = x->parent->right;
                    }

                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->right->color = BLACK;

                    leftRotate(x->parent);

                    x = root;
                }
            }
            else {

                Node* sibling = x->parent->left;

                if (sibling->color == RED) {

                    sibling->color = BLACK;
                    x->parent->color = RED;

                    rightRotate(x->parent);

                    sibling = x->parent->left;
                }

                if (sibling->right->color == BLACK &&
                    sibling->left->color == BLACK) {

                    sibling->color = RED;
                    x = x->parent;
                }
                else {

                    if (sibling->left->color == BLACK) {

                        sibling->right->color = BLACK;
                        sibling->color = RED;

                        leftRotate(sibling);

                        sibling = x->parent->left;
                    }

                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->left->color = BLACK;

                    rightRotate(x->parent);

                    x = root;
                }
            }
        }

        x->color = BLACK;
    }

    void destroy(Node* node) {
        if (node == NIL) {
            return;
        }

        destroy(node->left);
        destroy(node->right);

        delete node;
    }

public:

    RedBlackTreeIndex() {

        NIL = new Node();

        NIL->color = BLACK;
        NIL->left = NIL;
        NIL->right = NIL;
        NIL->parent = NIL;

        root = NIL;
    }

    ~RedBlackTreeIndex() override {
        destroy(root);
        delete NIL;
    }

    RedBlackTreeIndex(const RedBlackTreeIndex&) = delete;
    RedBlackTreeIndex& operator=(const RedBlackTreeIndex&) = delete;

    bool insert(const Place& place) override {

        if (!place.isValid()) {
            return false;
        }

        if (searchNode(place.id) != NIL) {
            return false;
        }

        Node* z = new Node(place);

        z->left = NIL;
        z->right = NIL;

        Node* parent = NIL;
        Node* current = root;

        while (current != NIL) {

            parent = current;

            if (z->place.id < current->place.id) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }

        z->parent = parent;

        if (parent == NIL) {
            root = z;
        }
        else if (z->place.id < parent->place.id) {
            parent->left = z;
        }
        else {
            parent->right = z;
        }

        insertFix(z);

        ++count;

        return true;
    }

    Place* search(int id) override {

        if (id <= 0) {
            return nullptr;
        }

        Node* result = searchNode(id);

        return result == NIL ? nullptr : &result->place;
    }

    bool remove(int id) override {

        Node* z = searchNode(id);

        if (z == NIL) {
            return false;
        }

        Node* y = z;
        Node* x;

        Color originalColor = y->color;

        if (z->left == NIL) {

            x = z->right;
            transplant(z, z->right);
        }
        else if (z->right == NIL) {

            x = z->left;
            transplant(z, z->left);
        }
        else {

            y = minimum(z->right);
            originalColor = y->color;

            x = y->right;

            if (y->parent == z) {
                x->parent = y;
            }
            else {
                transplant(y, y->right);

                y->right = z->right;
                y->right->parent = y;
            }

            transplant(z, y);

            y->left = z->left;
            y->left->parent = y;

            y->color = z->color;
        }

        delete z;

        if (originalColor == BLACK) {
            deleteFix(x);
        }

        --count;

        return true;
    }

    size_t size() const override {
        return count;
    }

    std::string name() const override {
        return "Red-Black Tree";
    }
};

} // namespace geosearch
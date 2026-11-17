
#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace geosearch {

template <typename T>
class Stack {

private:

    std::vector<T> elements;

public:

    void push(const T& value) {
        elements.push_back(value);
    }

    bool pop() {

        if (empty())
            return false;

        elements.pop_back();
        return true;
    }

    T& top() {

        if (empty())
            throw std::out_of_range("Stack is empty");

        return elements.back();
    }

    const T& top() const {

        if (empty())
            throw std::out_of_range("Stack is empty");

        return elements.back();
    }

    int search(const T& value) const {

        for (int i = static_cast<int>(elements.size()) - 1;
             i >= 0; --i) {

            if (elements[i] == value)
                return static_cast<int>(elements.size()) - 1 - i;
        }

        return -1;
    }

    size_t size() const {
        return elements.size();
    }

    bool empty() const {
        return elements.empty();
    }

    void clear() {
        elements.clear();
    }

    void display() const {

        std::cout << "TOP -> ";

        for (auto it = elements.rbegin();
             it != elements.rend(); ++it) {

            std::cout << *it << " ";
        }

        std::cout << "<- BOTTOM\n";
    }
};

} // namespace geosearch

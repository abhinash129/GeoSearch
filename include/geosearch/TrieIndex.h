
#pragma once

#include "Place.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace geosearch {

class TrieIndex {

private:

    struct Node {
        std::map<char, Node*> children;
        std::vector<Place> places;
        bool isEnd = false;
    };

    Node* root = new Node();
    size_t count = 0;
    std::unordered_set<int> insertedIds;

    std::string normalize(const std::string& text) const {

        std::string result;

        for (unsigned char ch : text) {
            result += static_cast<char>(std::tolower(ch));
        }

        return result;
    }

    void collect(
        Node* node,
        std::vector<Place>& results,
        size_t limit
    ) const {

        if (node == nullptr || results.size() >= limit)
            return;

        for (const Place& place : node->places) {

            if (results.size() >= limit)
                return;

            results.push_back(place);
        }

        for (const auto& entry : node->children) {

            if (results.size() >= limit)
                return;

            collect(entry.second, results, limit);
        }
    }

    void destroy(Node* node) {

        if (node == nullptr)
            return;

        for (const auto& entry : node->children)
            destroy(entry.second);

        delete node;
    }

    Node* findNode(const std::string& text) const {

        std::string key = normalize(text);

        Node* current = root;

        for (char ch : key) {

            auto it = current->children.find(ch);

            if (it == current->children.end())
                return nullptr;

            current = it->second;
        }

        return current;
    }

public:

    TrieIndex() = default;

    ~TrieIndex() {
        destroy(root);
    }

    TrieIndex(const TrieIndex&) = delete;
    TrieIndex& operator=(const TrieIndex&) = delete;

    bool insert(const Place& place) {

        if (!place.isValid() ||
            insertedIds.count(place.id) ||
            place.name.empty()) {
            return false;
        }

        std::string key = normalize(place.name);

        Node* current = root;

        for (char ch : key) {

            if (current->children.find(ch) ==
                current->children.end()) {

                current->children[ch] = new Node();
            }

            current = current->children[ch];
        }

        current->places.push_back(place);
        current->isEnd = true;

        insertedIds.insert(place.id);
        ++count;

        return true;
    }

    const Place* search(const std::string& name) const {

        Node* node = findNode(name);

        if (node == nullptr || !node->isEnd ||
            node->places.empty()) {
            return nullptr;
        }

        return &node->places.front();
    }

    bool startsWith(const std::string& prefix) const {

        Node* node = findNode(prefix);

        return node != nullptr;
    }

    std::vector<Place> autocomplete(
        const std::string& prefix,
        size_t limit = 10
    ) const {

        std::vector<Place> results;

        Node* node = findNode(prefix);

        if (node == nullptr || limit == 0)
            return results;

        collect(node, results, limit);

        return results;
    }

    size_t size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }
};

} // namespace geosearch


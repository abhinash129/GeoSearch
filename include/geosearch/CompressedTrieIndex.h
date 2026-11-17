
#pragma once

#include "Place.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

namespace geosearch {

class CompressedTrieIndex {

private:

    struct Node {
        std::string edge;
        std::vector<Place> places;
        std::vector<Node*> children;

        explicit Node(const std::string& value)
            : edge(value) {}
    };

    Node* root = new Node("");
    size_t count = 0;
    std::unordered_set<int> insertedIds;

    std::string normalize(const std::string& text) const {

        std::string result;

        for (unsigned char ch : text)
            result += static_cast<char>(std::tolower(ch));

        return result;
    }

    size_t commonPrefix(
        const std::string& first,
        const std::string& second
    ) const {

        size_t i = 0;

        while (i < first.size() &&
               i < second.size() &&
               first[i] == second[i]) {
            ++i;
        }

        return i;
    }

    void insertNode(
        Node* node,
        const std::string& key,
        size_t position,
        const Place& place
    ) {

        if (position == key.size()) {
            node->places.push_back(place);
            return;
        }

        for (Node* child : node->children) {

            size_t common = commonPrefix(
                child->edge,
                key.substr(position)
            );

            if (common == 0)
                continue;

            if (common == child->edge.size()) {

                insertNode(
                    child,
                    key,
                    position + common,
                    place
                );

                return;
            }

            Node* split = new Node(
                child->edge.substr(0, common)
            );

            child->edge = child->edge.substr(common);

            split->children.push_back(child);

            node->children.erase(
                std::remove(
                    node->children.begin(),
                    node->children.end(),
                    child
                ),
                node->children.end()
            );

            node->children.push_back(split);

            position += common;

            if (position == key.size()) {

                split->places.push_back(place);

            } else {

                Node* newChild = new Node(
                    key.substr(position)
                );

                newChild->places.push_back(place);

                split->children.push_back(newChild);
            }

            return;
        }

        Node* newChild = new Node(key.substr(position));

        newChild->places.push_back(place);

        node->children.push_back(newChild);
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

        for (Node* child : node->children) {

            if (results.size() >= limit)
                return;

            collect(child, results, limit);
        }
    }

    Node* findPrefixNode(
        Node* node,
        const std::string& prefix
    ) const {

        if (prefix.empty())
            return node;

        for (Node* child : node->children) {

            size_t common = commonPrefix(
                child->edge,
                prefix
            );

            if (common == prefix.size())
                return child;

            if (common == child->edge.size()) {

                return findPrefixNode(
                    child,
                    prefix.substr(common)
                );
            }
        }

        return nullptr;
    }

    void destroy(Node* node) {

        if (node == nullptr)
            return;

        for (Node* child : node->children)
            destroy(child);

        delete node;
    }

public:

    CompressedTrieIndex() = default;

    ~CompressedTrieIndex() {
        destroy(root);
    }

    CompressedTrieIndex(const CompressedTrieIndex&) = delete;
    CompressedTrieIndex& operator=(const CompressedTrieIndex&) = delete;

    bool insert(const Place& place) {

        if (!place.isValid() ||
            place.name.empty() ||
            insertedIds.count(place.id)) {
            return false;
        }

        std::string key = normalize(place.name);

        insertNode(root, key, 0, place);

        insertedIds.insert(place.id);
        ++count;

        return true;
    }

    const Place* search(const std::string& name) const {

        std::string key = normalize(name);

        Node* node = root;
        size_t position = 0;

        while (position < key.size()) {

            bool found = false;

            for (Node* child : node->children) {

                if (key.compare(
                        position,
                        child->edge.size(),
                        child->edge
                    ) == 0) {

                    position += child->edge.size();
                    node = child;
                    found = true;
                    break;
                }
            }

            if (!found)
                return nullptr;
        }

        return node->places.empty()
                   ? nullptr
                   : &node->places.front();
    }

    bool startsWith(const std::string& prefix) const {

        if (prefix.empty())
            return true;

        return findPrefixNode(
            root,
            normalize(prefix)
        ) != nullptr;
    }

    std::vector<Place> autocomplete(
        const std::string& prefix,
        size_t limit = 10
    ) const {

        std::vector<Place> results;

        if (limit == 0)
            return results;

        Node* node = findPrefixNode(
            root,
            normalize(prefix)
        );

        if (node != nullptr)
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

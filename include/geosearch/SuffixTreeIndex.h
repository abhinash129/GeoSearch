
#pragma once

#include "Place.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

namespace geosearch {

class SuffixTreeIndex {

private:

    struct Node {
        std::string edge;
        std::vector<size_t> documentIndices;
        std::vector<Node*> children;

        explicit Node(const std::string& value = "")
            : edge(value) {}
    };

    Node* root = new Node();

    std::vector<Place> places;
    std::vector<std::string> documents;

    std::unordered_set<int> insertedIds;

    bool built = false;

    std::string normalize(const std::string& text) const {

        std::string result;
        result.reserve(text.size());

        for (unsigned char ch : text) {
            result += static_cast<char>(std::tolower(ch));
        }

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

    void insertSuffix(
        Node* node,
        const std::string& suffix,
        size_t position,
        size_t documentIndex
    ) {

        if (position == suffix.size()) {
            node->documentIndices.push_back(documentIndex);
            return;
        }

        for (Node* child : node->children) {

            size_t common = commonPrefix(
                child->edge,
                suffix.substr(position)
            );

            if (common == 0)
                continue;

            if (common == child->edge.size()) {

                insertSuffix(
                    child,
                    suffix,
                    position + common,
                    documentIndex
                );

                return;
            }

            // Split the existing edge.
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

            if (position == suffix.size()) {

                split->documentIndices.push_back(documentIndex);

            } else {

                Node* newChild = new Node(
                    suffix.substr(position)
                );

                newChild->documentIndices.push_back(
                    documentIndex
                );

                split->children.push_back(newChild);
            }

            return;
        }

        // No matching edge: create a new edge.
        Node* newChild = new Node(
            suffix.substr(position)
        );

        newChild->documentIndices.push_back(documentIndex);

        node->children.push_back(newChild);
    }

    void build() {

        if (built)
            return;

        for (Node* child : root->children) {
            destroy(child);
        }

        root->children.clear();
        root->documentIndices.clear();

        // Insert every suffix of every document.
        for (size_t doc = 0; doc < documents.size(); ++doc) {

            const std::string& text = documents[doc];

            for (size_t offset = 0;
                 offset < text.size();
                 ++offset) {

                insertSuffix(
                    root,
                    text,
                    offset,
                    doc
                );
            }
        }

        built = true;
    }

    Node* findSubstringNode(
        Node* node,
        const std::string& query
    ) const {

        if (query.empty())
            return node;

        for (Node* child : node->children) {

            size_t common = commonPrefix(
                child->edge,
                query
            );

            if (common == 0)
                continue;

            // Query ends inside or exactly at this edge.
            if (common == query.size()) {
                return child;
            }

            // Entire edge matches; continue searching.
            if (common == child->edge.size()) {

                return findSubstringNode(
                    child,
                    query.substr(common)
                );
            }
        }

        return nullptr;
    }

    void collectDocuments(
        Node* node,
        std::unordered_set<size_t>& results
    ) const {

        if (node == nullptr)
            return;

        for (size_t index : node->documentIndices) {
            results.insert(index);
        }

        for (Node* child : node->children) {
            collectDocuments(child, results);
        }
    }

    void destroy(Node* node) {

        if (node == nullptr)
            return;

        for (Node* child : node->children) {
            destroy(child);
        }

        delete node;
    }

public:

    SuffixTreeIndex() = default;

    ~SuffixTreeIndex() {
        destroy(root);
    }

    SuffixTreeIndex(const SuffixTreeIndex&) = delete;
    SuffixTreeIndex& operator=(const SuffixTreeIndex&) = delete;

    bool insert(const Place& place) {

        if (!place.isValid() ||
            insertedIds.count(place.id)) {
            return false;
        }

        std::string document = normalize(
            place.name + " " + place.description
        );

        places.push_back(place);
        documents.push_back(document);

        insertedIds.insert(place.id);

        built = false;

        return true;
    }

    std::vector<Place> searchSubstring(
        const std::string& query
    ) {

        std::vector<Place> results;

        std::string normalizedQuery = normalize(query);

        if (normalizedQuery.empty())
            return results;

        build();

        Node* match = findSubstringNode(
            root,
            normalizedQuery
        );

        if (match == nullptr)
            return results;

        std::unordered_set<size_t> documentMatches;

        collectDocuments(match, documentMatches);

        for (size_t index : documentMatches) {
            results.push_back(places[index]);
        }

        return results;
    }

    size_t size() const {
        return places.size();
    }

    bool empty() const {
        return places.empty();
    }
};

} // namespace geosearch

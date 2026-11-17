
#pragma once

#include "Place.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace geosearch {

class SuffixArrayIndex {

private:

    struct Suffix {
        size_t documentIndex;
        size_t offset;
    };

    std::vector<Place> places;
    std::vector<std::string> documents;
    std::vector<Suffix> suffixes;

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

    std::string_view getSuffix(const Suffix& suffix) const {

        const std::string& document =
            documents[suffix.documentIndex];

        return std::string_view(
            document.data() + suffix.offset,
            document.size() - suffix.offset
        );
    }

    void build() {

        if (built)
            return;

        suffixes.clear();

        // Generate every suffix.
        for (size_t doc = 0; doc < documents.size(); ++doc) {

            for (size_t offset = 0;
                 offset < documents[doc].size();
                 ++offset) {

                suffixes.push_back({doc, offset});
            }
        }

        // Sort suffixes lexicographically.
        std::sort(
            suffixes.begin(),
            suffixes.end(),
            [this](const Suffix& a, const Suffix& b) {

                return getSuffix(a) < getSuffix(b);
            }
        );

        built = true;
    }

public:

    bool insert(const Place& place) {

        if (!place.isValid() ||
            insertedIds.count(place.id)) {
            return false;
        }

        // Index both name and description.
        std::string document =
            normalize(place.name + " " + place.description);

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

        std::string_view target(normalizedQuery);

        // Find the first suffix >= query.
        auto it = std::lower_bound(
            suffixes.begin(),
            suffixes.end(),
            target,
            [this](const Suffix& suffix,
                   std::string_view value) {

                return getSuffix(suffix) < value;
            }
        );

        std::unordered_set<int> foundIds;

        // All suffixes beginning with the query are consecutive.
        while (it != suffixes.end()) {

            std::string_view current = getSuffix(*it);

            if (current.size() < target.size() ||
                current.substr(0, target.size()) != target) {
                break;
            }

            size_t documentIndex = it->documentIndex;

            const Place& place = places[documentIndex];

            if (foundIds.insert(place.id).second) {
                results.push_back(place);
            }

            ++it;
        }

        return results;
    }

    size_t size() const {
        return places.size();
    }

    size_t suffixCount() {

        build();
        return suffixes.size();
    }

    bool empty() const {
        return places.empty();
    }
};

} // namespace geosearch

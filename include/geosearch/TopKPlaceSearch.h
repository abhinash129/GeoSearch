
#pragma once

#include "Place.h"

#include <vector>
#include <queue>
#include <algorithm>

namespace geosearch {

class TopKPlaceSearch {

private:

    struct RatingComparator {

        bool operator()(
            const Place& a,
            const Place& b
        ) const {

            if (a.rating != b.rating) {
                return a.rating < b.rating;
            }

            return a.id > b.id;
        }
    };

public:

    std::vector<Place> findTopK(
        const std::vector<Place>& places,
        size_t k
    ) const {

        if (k == 0 || places.empty()) {
            return {};
        }

        std::priority_queue<
            Place,
            std::vector<Place>,
            RatingComparator
        > maxHeap;

        for (const auto& place : places) {
            maxHeap.push(place);
        }

        std::vector<Place> results;

        size_t count = std::min(k, places.size());

        for (size_t i = 0; i < count; ++i) {

            results.push_back(maxHeap.top());
            maxHeap.pop();
        }

        return results;
    }
};

} // namespace geosearch

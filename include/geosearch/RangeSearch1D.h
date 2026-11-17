
#pragma once

#include "Place.h"

#include <vector>
#include <algorithm>
#include <stdexcept>

namespace geosearch {

class RangeSearch1D {

private:

    struct Entry {
        double value;
        Place place;
    };

    std::vector<Entry> entries;
    bool sorted = false;

    void buildIndex() {

        if (sorted) {
            return;
        }

        std::sort(
            entries.begin(),
            entries.end(),
            [](const Entry& a, const Entry& b) {
                if (a.value == b.value) {
                    return a.place.id < b.place.id;
                }

                return a.value < b.value;
            }
        );

        sorted = true;
    }

public:

    void insert(const Place& place, double value) {

        if (value < 0.0) {
            throw std::invalid_argument(
                "Range index value cannot be negative"
            );
        }

        entries.push_back({value, place});
        sorted = false;
    }

    std::vector<Place> searchRange(
        double minimum,
        double maximum
    ) {

        if (minimum > maximum) {
            throw std::invalid_argument(
                "Minimum cannot exceed maximum"
            );
        }

        buildIndex();

        auto first = std::lower_bound(
            entries.begin(),
            entries.end(),
            minimum,
            [](const Entry& entry, double value) {
                return entry.value < value;
            }
        );

        auto last = std::upper_bound(
            entries.begin(),
            entries.end(),
            maximum,
            [](double value, const Entry& entry) {
                return value < entry.value;
            }
        );

        std::vector<Place> results;

        for (auto it = first; it != last; ++it) {
            results.push_back(it->place);
        }

        return results;
    }

    size_t size() const {
        return entries.size();
    }

    void clear() {
        entries.clear();
        sorted = false;
    }
};

} // namespace geosearch

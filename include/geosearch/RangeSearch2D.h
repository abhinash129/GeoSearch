
#pragma once

#include "Place.h"

#include <vector>
#include <algorithm>
#include <stdexcept>

namespace geosearch {

class RangeSearch2D {

private:

    std::vector<Place> places;

    bool built = false;

    std::vector<Place> sortedByLatitude;

public:

    void insert(const Place& place) {

        places.push_back(place);
        built = false;
    }

    void build() {

        sortedByLatitude = places;

        std::sort(
            sortedByLatitude.begin(),
            sortedByLatitude.end(),
            [](const Place& a, const Place& b) {
                if (a.latitude == b.latitude) {
                    return a.longitude < b.longitude;
                }

                return a.latitude < b.latitude;
            }
        );

        built = true;
    }

    std::vector<Place> searchRange(
        double minLatitude,
        double maxLatitude,
        double minLongitude,
        double maxLongitude
    ) {

        if (minLatitude > maxLatitude ||
            minLongitude > maxLongitude) {

            throw std::invalid_argument(
                "Invalid geographic range"
            );
        }

        if (!built) {
            build();
        }

        auto first = std::lower_bound(
            sortedByLatitude.begin(),
            sortedByLatitude.end(),
            minLatitude,
            [](const Place& place, double value) {
                return place.latitude < value;
            }
        );

        auto last = std::upper_bound(
            sortedByLatitude.begin(),
            sortedByLatitude.end(),
            maxLatitude,
            [](double value, const Place& place) {
                return value < place.latitude;
            }
        );

        std::vector<Place> results;

        for (auto it = first; it != last; ++it) {

            if (it->longitude >= minLongitude &&
                it->longitude <= maxLongitude) {

                results.push_back(*it);
            }
        }

        return results;
    }

    size_t size() const {
        return places.size();
    }

    void clear() {

        places.clear();
        sortedByLatitude.clear();
        built = false;
    }
};

} // namespace geosearch

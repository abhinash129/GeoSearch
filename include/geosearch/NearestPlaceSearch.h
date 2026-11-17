
#pragma once

#include "Place.h"

#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace geosearch {

class NearestPlaceSearch {

private:

    static constexpr double EARTH_RADIUS_KM = 6371.0;

    std::vector<Place> places;

    static double toRadians(double degrees) {
        return degrees * 3.14159265358979323846 / 180.0;
    }

    struct HeapEntry {
        double distance;
        Place place;
    };

    // Compare only distances. Never compare Place objects.
    struct DistanceComparator {

        bool operator()(
            const HeapEntry& a,
            const HeapEntry& b
        ) const {
            return a.distance < b.distance;
        }
    };

public:

    void build(const std::vector<Place>& inputPlaces) {
        places = inputPlaces;
    }

    static double calculateDistance(
        double lat1,
        double lon1,
        double lat2,
        double lon2
    ) {

        double dLat = toRadians(lat2 - lat1);
        double dLon = toRadians(lon2 - lon1);

        lat1 = toRadians(lat1);
        lat2 = toRadians(lat2);

        double a =
            std::sin(dLat / 2) * std::sin(dLat / 2) +
            std::cos(lat1) * std::cos(lat2) *
            std::sin(dLon / 2) * std::sin(dLon / 2);

        double c = 2 * std::atan2(
            std::sqrt(a),
            std::sqrt(1.0 - a)
        );

        return EARTH_RADIUS_KM * c;
    }

    std::vector<std::pair<Place, double>> nearest(
        double latitude,
        double longitude,
        size_t k
    ) const {

        if (latitude < -90.0 || latitude > 90.0 ||
            longitude < -180.0 || longitude > 180.0) {

            throw std::invalid_argument(
                "Invalid search coordinates"
            );
        }

        if (k == 0 || places.empty()) {
            return {};
        }

        std::priority_queue<
            HeapEntry,
            std::vector<HeapEntry>,
            DistanceComparator
        > maxHeap;

        for (const auto& place : places) {

            double distance = calculateDistance(
                latitude,
                longitude,
                place.latitude,
                place.longitude
            );

            if (maxHeap.size() < k) {

                maxHeap.push({distance, place});

            } else if (distance < maxHeap.top().distance) {

                maxHeap.pop();
                maxHeap.push({distance, place});
            }
        }

        std::vector<std::pair<Place, double>> results;

        while (!maxHeap.empty()) {

            results.push_back({
                maxHeap.top().place,
                maxHeap.top().distance
            });

            maxHeap.pop();
        }

        std::reverse(results.begin(), results.end());

        return results;
    }

    size_t size() const {
        return places.size();
    }
};

} // namespace geosearch

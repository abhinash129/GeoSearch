
#pragma once

#include "Place.h"
#include "NearestPlaceSearch.h"
#include "TopKPlaceSearch.h"
#include "RecommendationSearch.h"

#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <utility>

namespace geosearch {

class SearchService {

private:

    std::vector<Place> places;

    static std::string toLower(const std::string& value) {

        std::string result = value;

        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );

        return result;
    }

public:

    void build(const std::vector<Place>& input) {
        places = input;
    }

    size_t size() const {
        return places.size();
    }

    // 1. Search by ID
    bool searchById(int id, Place& result) const {

        for (const auto& place : places) {

            if (place.id == id) {
                result = place;
                return true;
            }
        }

        return false;
    }

    // 2. Search by name or description
    std::vector<Place> searchByKeyword(
        const std::string& keyword
    ) const {

        std::vector<Place> results;

        std::string key = toLower(keyword);

        if (key.empty()) {
            return results;
        }

        for (const auto& place : places) {

            std::string name = toLower(place.name);
            std::string description =
                toLower(place.description);

            if (name.find(key) != std::string::npos ||
                description.find(key) != std::string::npos) {

                results.push_back(place);
            }
        }

        return results;
    }

    // 3. Filter by category
    std::vector<Place> searchByCategory(
        const std::string& category
    ) const {

        std::vector<Place> results;

        std::string target = toLower(category);

        for (const auto& place : places) {

            if (toLower(place.category) == target) {
                results.push_back(place);
            }
        }

        return results;
    }

    // 4. Filter by rating
    std::vector<Place> filterByRating(
        double minimum,
        double maximum
    ) const {

        std::vector<Place> results;

        if (minimum > maximum) {
            return results;
        }

        for (const auto& place : places) {

            if (place.hasRating() &&
                place.rating >= minimum &&
                place.rating <= maximum) {

                results.push_back(place);
            }
        }

        return results;
    }

    // 5. Filter by price
    std::vector<Place> filterByPrice(
        double minimum,
        double maximum
    ) const {

        std::vector<Place> results;

        if (minimum > maximum) {
            return results;
        }

        for (const auto& place : places) {

            if (place.hasPrice() &&
                place.price >= minimum &&
                place.price <= maximum) {

                results.push_back(place);
            }
        }

        return results;
    }

    // 6. Geographic map-window search
    std::vector<Place> searchMapWindow(
        double minLatitude,
        double maxLatitude,
        double minLongitude,
        double maxLongitude,
        double minimumRating = 0.0
    ) const {

        std::vector<Place> results;

        if (minLatitude > maxLatitude ||
            minLongitude > maxLongitude) {
            return results;
        }

        for (const auto& place : places) {

            if (place.latitude >= minLatitude &&
                place.latitude <= maxLatitude &&
                place.longitude >= minLongitude &&
                place.longitude <= maxLongitude &&
                place.rating >= minimumRating) {

                results.push_back(place);
            }
        }

        return results;
    }

    // 7. Find nearest places
    std::vector<std::pair<Place, double>> findNearest(
        double latitude,
        double longitude,
        size_t k
    ) const {

        NearestPlaceSearch search;

        search.build(places);

        return search.nearest(latitude, longitude, k);
    }

    // 8. Find highest-rated places
    std::vector<Place> findTopRated(size_t k) const {

        TopKPlaceSearch search;

        return search.findTopK(places, k);
    }

    // 9. Find best recommended places
    std::vector<RecommendedPlace> recommendPlaces(
        double latitude,
        double longitude,
        const std::string& category,
        double radiusKm,
        size_t limit,
        RecommendationWeights weights = {}
    ) const {

        return RecommendationSearch::recommend(
            places,
            latitude,
            longitude,
            category,
            radiusKm,
            limit,
            weights
        );
    }
};

} // namespace geosearch

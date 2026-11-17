
#pragma once

#include "Place.h"
#include "NearestPlaceSearch.h"

#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace geosearch {

struct RecommendationWeights {
    double distance = 0.40;
    double rating = 0.40;
    double price = 0.20;
};

struct RecommendedPlace {
    Place place;

    double distanceKm = 0.0;

    // -1 means this score is unavailable.
    double distanceScore = 0.0;
    double ratingScore = -1.0;
    double priceScore = -1.0;

    double overallScore = 0.0;
};

class RecommendationSearch {

private:

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

    static std::vector<RecommendedPlace> recommend(
        const std::vector<Place>& places,
        double latitude,
        double longitude,
        const std::string& category,
        double radiusKm,
        size_t limit,
        RecommendationWeights weights = {}
    ) {

        // Validate coordinates.
        if (!std::isfinite(latitude) ||
            !std::isfinite(longitude) ||
            latitude < -90.0 ||
            latitude > 90.0 ||
            longitude < -180.0 ||
            longitude > 180.0) {

            throw std::invalid_argument(
                "Invalid location coordinates"
            );
        }

        // Validate radius.
        if (!std::isfinite(radiusKm) ||
            radiusKm <= 0.0 ||
            radiusKm > 1000.0) {

            throw std::invalid_argument(
                "Radius must be between 0 and 1000 km"
            );
        }

        if (limit < 1 || limit > 100) {
            throw std::invalid_argument(
                "Limit must be between 1 and 100"
            );
        }

        // Validate weights.
        if (!std::isfinite(weights.distance) ||
            !std::isfinite(weights.rating) ||
            !std::isfinite(weights.price) ||
            weights.distance < 0.0 ||
            weights.rating < 0.0 ||
            weights.price < 0.0) {

            throw std::invalid_argument(
                "Recommendation weights must be non-negative"
            );
        }

        double totalWeight =
            weights.distance +
            weights.rating +
            weights.price;

        if (!std::isfinite(totalWeight) ||
            totalWeight <= 0.0) {

            throw std::invalid_argument(
                "At least one recommendation weight must be positive"
            );
        }

        // Normalize configured weights.
        weights.distance /= totalWeight;
        weights.rating /= totalWeight;
        weights.price /= totalWeight;

        const std::string selectedCategory =
            toLower(category);

        std::vector<RecommendedPlace> results;

        // Step 1: Filter by category and radius.
        for (const auto& place : places) {

            if (!selectedCategory.empty() &&
                toLower(place.category) != selectedCategory) {
                continue;
            }

            double distance =
                NearestPlaceSearch::calculateDistance(
                    latitude,
                    longitude,
                    place.latitude,
                    place.longitude
                );

            if (distance > radiusKm) {
                continue;
            }

            RecommendedPlace item;

            item.place = place;
            item.distanceKm = distance;

            item.distanceScore =
                1.0 - (distance / radiusKm);

            if (place.hasRating()) {
                item.ratingScore =
                    place.rating / 5.0;
            }

            results.push_back(item);
        }

        if (results.empty()) {
            return results;
        }

        // Step 2: Find price range using only known prices.
        double minimumPrice = 0.0;
        double maximumPrice = 0.0;

        bool hasKnownPrice = false;

        for (const auto& item : results) {

            if (!item.place.hasPrice()) {
                continue;
            }

            if (!hasKnownPrice) {
                minimumPrice = item.place.price;
                maximumPrice = item.place.price;
                hasKnownPrice = true;
            } else {
                minimumPrice = std::min(
                    minimumPrice,
                    item.place.price
                );

                maximumPrice = std::max(
                    maximumPrice,
                    item.place.price
                );
            }
        }

        // Step 3: Calculate available price scores.
        for (auto& item : results) {

            if (!item.place.hasPrice()) {
                item.priceScore = -1.0;
                continue;
            }

            if (maximumPrice == minimumPrice) {
                item.priceScore = 1.0;
            } else {
                item.priceScore =
                    1.0 -
                    (item.place.price - minimumPrice) /
                    (maximumPrice - minimumPrice);
            }
        }

        // Step 4: Calculate overall score.
        // Missing criteria do not contribute to the denominator.
        for (auto& item : results) {

            double effectiveWeight = weights.distance;

            double weightedScore =
                weights.distance * item.distanceScore;

            if (item.place.hasRating()) {

                effectiveWeight += weights.rating;

                weightedScore +=
                    weights.rating * item.ratingScore;
            }

            if (item.place.hasPrice()) {

                effectiveWeight += weights.price;

                weightedScore +=
                    weights.price * item.priceScore;
            }

            if (effectiveWeight > 0.0) {

                item.overallScore =
                    weightedScore / effectiveWeight;

            } else {

                // Fallback when all configured weighted criteria
                // are unavailable for this particular place.
                item.overallScore = item.distanceScore;
            }
        }

        // Step 5: Sort recommendations.
        std::sort(
            results.begin(),
            results.end(),
            [](const RecommendedPlace& a,
               const RecommendedPlace& b) {

                if (a.overallScore != b.overallScore) {
                    return a.overallScore > b.overallScore;
                }

                if (a.distanceKm != b.distanceKm) {
                    return a.distanceKm < b.distanceKm;
                }

                // Prefer known ratings when scores are tied.
                if (a.place.hasRating() != b.place.hasRating()) {
                    return a.place.hasRating();
                }

                if (a.place.hasRating() &&
                    a.place.rating != b.place.rating) {
                    return a.place.rating > b.place.rating;
                }

                // Prefer known prices when scores are tied.
                if (a.place.hasPrice() != b.place.hasPrice()) {
                    return a.place.hasPrice();
                }

                if (a.place.hasPrice() &&
                    a.place.price != b.place.price) {
                    return a.place.price < b.place.price;
                }

                return a.place.id < b.place.id;
            }
        );

        // Step 6: Limit results.
        if (results.size() > limit) {
            results.resize(limit);
        }

        return results;
    }
};

} // namespace geosearch


#pragma once

#include "Place.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace geosearch {

class PrioritySearchTree {

private:

    struct Node {

        Place best;

        double minLongitude;
        double maxLongitude;
        double maxRating;

        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        explicit Node(const Place& place)
            : best(place),
              minLongitude(place.longitude),
              maxLongitude(place.longitude),
              maxRating(place.rating) {
        }
    };

    std::unique_ptr<Node> root;

    std::unique_ptr<Node> buildTree(
        std::vector<Place> points
    ) {

        if (points.empty()) {
            return nullptr;
        }

        // Find the place with the highest rating.
        auto bestIt = std::max_element(
            points.begin(),
            points.end(),
            [](const Place& a, const Place& b) {
                return a.rating < b.rating;
            }
        );

        Place bestPlace = *bestIt;

        points.erase(bestIt);

        auto node = std::make_unique<Node>(bestPlace);

        if (points.empty()) {
            return node;
        }

        // Sort remaining places by longitude.
        std::sort(
            points.begin(),
            points.end(),
            [](const Place& a, const Place& b) {
                return a.longitude < b.longitude;
            }
        );

        // Split remaining places into two balanced groups.
        size_t middle = points.size() / 2;

        std::vector<Place> leftPoints(
            points.begin(),
            points.begin() + middle
        );

        std::vector<Place> rightPoints(
            points.begin() + middle,
            points.end()
        );

        node->left = buildTree(std::move(leftPoints));
        node->right = buildTree(std::move(rightPoints));

        // Store the longitude boundaries and maximum rating
        // of the entire subtree.
        if (node->left) {

            node->minLongitude = std::min(
                node->minLongitude,
                node->left->minLongitude
            );

            node->maxLongitude = std::max(
                node->maxLongitude,
                node->left->maxLongitude
            );

            node->maxRating = std::max(
                node->maxRating,
                node->left->maxRating
            );
        }

        if (node->right) {

            node->minLongitude = std::min(
                node->minLongitude,
                node->right->minLongitude
            );

            node->maxLongitude = std::max(
                node->maxLongitude,
                node->right->maxLongitude
            );

            node->maxRating = std::max(
                node->maxRating,
                node->right->maxRating
            );
        }

        return node;
    }

    void searchTree(
        const Node* node,
        double minLatitude,
        double maxLatitude,
        double minLongitude,
        double maxLongitude,
        double minRating,
        std::vector<Place>& results
    ) const {

        if (!node) {
            return;
        }

        // Prune subtrees that cannot contain a result.
        if (node->maxRating < minRating ||
            node->maxLongitude < minLongitude ||
            node->minLongitude > maxLongitude) {

            return;
        }

        const Place& place = node->best;

        // Check all query conditions.
        if (place.longitude >= minLongitude &&
            place.longitude <= maxLongitude &&
            place.latitude >= minLatitude &&
            place.latitude <= maxLatitude &&
            place.rating >= minRating) {

            results.push_back(place);
        }

        searchTree(
            node->left.get(),
            minLatitude,
            maxLatitude,
            minLongitude,
            maxLongitude,
            minRating,
            results
        );

        searchTree(
            node->right.get(),
            minLatitude,
            maxLatitude,
            minLongitude,
            maxLongitude,
            minRating,
            results
        );
    }

public:

    void build(const std::vector<Place>& places) {

        root = buildTree(places);
    }

    std::vector<Place> search(
        double minLatitude,
        double maxLatitude,
        double minLongitude,
        double maxLongitude,
        double minRating
    ) const {

        if (minLatitude > maxLatitude ||
            minLongitude > maxLongitude ||
            minRating < 0.0 ||
            minRating > 5.0) {

            throw std::invalid_argument(
                "Invalid Priority Search Tree query"
            );
        }

        std::vector<Place> results;

        searchTree(
            root.get(),
            minLatitude,
            maxLatitude,
            minLongitude,
            maxLongitude,
            minRating,
            results
        );

        return results;
    }

    bool empty() const {
        return root == nullptr;
    }
};

} // namespace geosearch

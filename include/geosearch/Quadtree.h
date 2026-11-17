
#pragma once

#include "Place.h"

#include <array>
#include <memory>
#include <vector>
#include <stdexcept>

namespace geosearch {

class Quadtree {

private:

    struct Node {

        double minLatitude;
        double maxLatitude;
        double minLongitude;
        double maxLongitude;

        int depth;

        std::vector<Place> places;

        std::array<std::unique_ptr<Node>, 4> children;

        Node(
            double minLat,
            double maxLat,
            double minLon,
            double maxLon,
            int nodeDepth
        )
            : minLatitude(minLat),
              maxLatitude(maxLat),
              minLongitude(minLon),
              maxLongitude(maxLon),
              depth(nodeDepth) {
        }

        bool isLeaf() const {
            for (const auto& child : children) {
                if (child) {
                    return false;
                }
            }

            return true;
        }
    };

    std::unique_ptr<Node> root;

    size_t capacity;
    int maxDepth;

    void subdivide(Node* node) {

        double midLatitude =
            (node->minLatitude + node->maxLatitude) / 2.0;

        double midLongitude =
            (node->minLongitude + node->maxLongitude) / 2.0;

        int nextDepth = node->depth + 1;

        // 0: Northwest
        node->children[0] = std::make_unique<Node>(
            midLatitude,
            node->maxLatitude,
            node->minLongitude,
            midLongitude,
            nextDepth
        );

        // 1: Northeast
        node->children[1] = std::make_unique<Node>(
            midLatitude,
            node->maxLatitude,
            midLongitude,
            node->maxLongitude,
            nextDepth
        );

        // 2: Southwest
        node->children[2] = std::make_unique<Node>(
            node->minLatitude,
            midLatitude,
            node->minLongitude,
            midLongitude,
            nextDepth
        );

        // 3: Southeast
        node->children[3] = std::make_unique<Node>(
            node->minLatitude,
            midLatitude,
            midLongitude,
            node->maxLongitude,
            nextDepth
        );

        std::vector<Place> existingPlaces;

        existingPlaces.swap(node->places);

        for (const auto& place : existingPlaces) {
            insertIntoNode(node, place);
        }
    }

    int getQuadrant(
        const Node* node,
        const Place& place
    ) const {

        double midLatitude =
            (node->minLatitude + node->maxLatitude) / 2.0;

        double midLongitude =
            (node->minLongitude + node->maxLongitude) / 2.0;

        bool north = place.latitude >= midLatitude;
        bool east = place.longitude >= midLongitude;

        if (north && !east) return 0;
        if (north && east) return 1;
        if (!north && !east) return 2;

        return 3;
    }

    void insertIntoNode(Node* node, const Place& place) {

        if (node->isLeaf()) {

            if (node->places.size() < capacity ||
                node->depth >= maxDepth) {

                node->places.push_back(place);
                return;
            }

            subdivide(node);
        }

        int quadrant = getQuadrant(node, place);

        insertIntoNode(
            node->children[quadrant].get(),
            place
        );
    }

    bool intersects(
        const Node* node,
        double minLat,
        double maxLat,
        double minLon,
        double maxLon
    ) const {

        return !(
            node->maxLatitude < minLat ||
            node->minLatitude > maxLat ||
            node->maxLongitude < minLon ||
            node->minLongitude > maxLon
        );
    }

    void searchNode(
        const Node* node,
        double minLat,
        double maxLat,
        double minLon,
        double maxLon,
        std::vector<Place>& results
    ) const {

        if (!node || !intersects(
                node, minLat, maxLat, minLon, maxLon)) {
            return;
        }

        for (const auto& place : node->places) {

            if (place.latitude >= minLat &&
                place.latitude <= maxLat &&
                place.longitude >= minLon &&
                place.longitude <= maxLon) {

                results.push_back(place);
            }
        }

        for (const auto& child : node->children) {

            if (child) {
                searchNode(
                    child.get(),
                    minLat,
                    maxLat,
                    minLon,
                    maxLon,
                    results
                );
            }
        }
    }

public:

    explicit Quadtree(
        double minLat,
        double maxLat,
        double minLon,
        double maxLon,
        size_t nodeCapacity = 4,
        int maximumDepth = 12
    )
        : capacity(nodeCapacity),
          maxDepth(maximumDepth) {

        if (minLat >= maxLat ||
            minLon >= maxLon ||
            minLat < -90.0 ||
            maxLat > 90.0 ||
            minLon < -180.0 ||
            maxLon > 180.0 ||
            nodeCapacity == 0 ||
            maximumDepth < 0) {

            throw std::invalid_argument(
                "Invalid Quadtree configuration"
            );
        }

        root = std::make_unique<Node>(
            minLat,
            maxLat,
            minLon,
            maxLon,
            0
        );
    }

    bool insert(const Place& place) {

        if (place.latitude < root->minLatitude ||
            place.latitude > root->maxLatitude ||
            place.longitude < root->minLongitude ||
            place.longitude > root->maxLongitude) {

            return false;
        }

        insertIntoNode(root.get(), place);

        return true;
    }

    std::vector<Place> searchRange(
        double minLat,
        double maxLat,
        double minLon,
        double maxLon
    ) const {

        if (minLat > maxLat || minLon > maxLon) {
            throw std::invalid_argument(
                "Invalid geographic search range"
            );
        }

        std::vector<Place> results;

        searchNode(
            root.get(),
            minLat,
            maxLat,
            minLon,
            maxLon,
            results
        );

        return results;
    }
};

} // namespace geosearch

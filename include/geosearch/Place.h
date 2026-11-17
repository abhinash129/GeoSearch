
#pragma once

#include <string>
#include <cmath>

namespace geosearch {

struct Place {
    int id = 0;

    // Stable identity from OpenStreetMap.
    std::string osmType;
    std::string osmId;

    std::string name;
    std::string category;
    std::string description;

    double latitude = 0.0;
    double longitude = 0.0;

    // -1 means the information is unavailable.
    double rating = -1.0;
    double price = -1.0;

    // Optional information collected from OpenStreetMap.
    std::string cuisine;
    std::string openingHours;
    std::string phone;
    std::string website;
    std::string brand;
    std::string operatorName;
    std::string address;

    bool hasRating() const {
        return std::isfinite(rating) &&
               rating >= 0.0 &&
               rating <= 5.0;
    }

    bool hasPrice() const {
        return std::isfinite(price) &&
               price >= 0.0;
    }

    bool isValid() const {
        return id > 0 &&
               !name.empty() &&
               std::isfinite(latitude) &&
               std::isfinite(longitude) &&
               latitude >= -90.0 &&
               latitude <= 90.0 &&
               longitude >= -180.0 &&
               longitude <= 180.0 &&
               (rating == -1.0 || hasRating()) &&
               (price == -1.0 || hasPrice());
    }
};

} // namespace geosearch

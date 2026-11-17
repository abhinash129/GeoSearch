
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "geosearch/Place.h"
#include "geosearch/CSVLoader.h"
#include "geosearch/SearchService.h"

using json = nlohmann::json;


namespace {

// Convert one Place into JSON.
json placeToJson(const geosearch::Place& place) {

    return {
        {"id", place.id},
        {"name", place.name},
        {"category", place.category},
        {"description", place.description},
        {"latitude", place.latitude},
        {"longitude", place.longitude},
        {"rating", place.hasRating()
            ? json(place.rating)
            : json(nullptr)},
        {"price", place.hasPrice()
            ? json(place.price)
            : json(nullptr)},

        // Additional OpenStreetMap attributes.
        {"cuisine", place.cuisine},
        {"openingHours", place.openingHours},
        {"phone", place.phone},
        {"website", place.website},
        {"brand", place.brand},
        {"operator", place.operatorName},
        {"address", place.address}
    };
}

// Convert multiple places into a JSON array.
json placesToJson(
    const std::vector<geosearch::Place>& places
) {

    json result = json::array();

    for (const auto& place : places) {
        result.push_back(placeToJson(place));
    }

    return result;
}

void sendError(
    httplib::Response& response,
    int status,
    const std::string& message
) {

    response.status = status;

    response.set_content(
        json({
            {"success", false},
            {"error", message}
        }).dump(4),
        "application/json"
    );
}

double getDoubleParam(
    const httplib::Request& request,
    const std::string& name
) {

    if (!request.has_param(name)) {
        throw std::invalid_argument(
            "Missing parameter: " + name
        );
    }

    return std::stod(request.get_param_value(name));
}

int getIntParam(
    const httplib::Request& request,
    const std::string& name
) {

    if (!request.has_param(name)) {
        throw std::invalid_argument(
            "Missing parameter: " + name
        );
    }

    return std::stoi(request.get_param_value(name));
}

} // namespace


int main() {

    try {

        std::cout << "====================================\n";
        std::cout << "       GEOSEARCH HTTP SERVER\n";
        std::cout << "====================================\n";

        // Load dataset
        std::vector<geosearch::Place> places =
            geosearch::CSVLoader::load("data/places.csv");

        // Build integrated search service
        geosearch::SearchService searchService;

        searchService.build(places);

        std::cout << "\nSearch service initialized.\n";
        std::cout << "Indexed places: "
                  << searchService.size() << "\n";

        httplib::Server server;

        // Health endpoint
        server.Get(
            "/api/health",
            [](const httplib::Request&,
               httplib::Response& response) {

                response.set_content(
                    json({
                        {"success", true},
                        {"service", "GeoSearch"},
                        {"status", "running"}
                    }).dump(4),
                    "application/json"
                );
            }
        );

        // Get all places
        server.Get(
            "/api/places",
            [&](const httplib::Request&,
                httplib::Response& response) {

                response.set_content(
                    json({
                        {"success", true},
                        {"count", places.size()},
                        {"data", placesToJson(places)}
                    }).dump(4),
                    "application/json"
                );
            }
        );

        // Search by ID
        server.Get(
            "/api/place",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    int id = getIntParam(request, "id");

                    geosearch::Place place;

                    if (!searchService.searchById(id, place)) {

                        sendError(
                            response,
                            404,
                            "Place not found"
                        );

                        return;
                    }

                    response.set_content(
                        json({
                            {"success", true},
                            {"data", placeToJson(place)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Keyword search
        server.Get(
            "/api/search",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    if (!request.has_param("keyword")) {
                        throw std::invalid_argument(
                            "Missing parameter: keyword"
                        );
                    }

                    std::string keyword =
                        request.get_param_value("keyword");

                    auto results =
                        searchService.searchByKeyword(keyword);

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", results.size()},
                            {"data", placesToJson(results)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Category search
        server.Get(
            "/api/category",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    if (!request.has_param("name")) {
                        throw std::invalid_argument(
                            "Missing parameter: name"
                        );
                    }

                    auto results =
                        searchService.searchByCategory(
                            request.get_param_value("name")
                        );

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", results.size()},
                            {"data", placesToJson(results)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Rating range
        server.Get(
            "/api/rating",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    double minimum =
                        getDoubleParam(request, "min");

                    double maximum =
                        getDoubleParam(request, "max");

                    if (minimum < 0 || maximum > 5 ||
                        minimum > maximum) {

                        throw std::invalid_argument(
                            "Invalid rating range"
                        );
                    }

                    auto results =
                        searchService.filterByRating(
                            minimum,
                            maximum
                        );

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", results.size()},
                            {"data", placesToJson(results)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Price range
        server.Get(
            "/api/price",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    double minimum =
                        getDoubleParam(request, "min");

                    double maximum =
                        getDoubleParam(request, "max");

                    if (minimum < 0 || minimum > maximum) {

                        throw std::invalid_argument(
                            "Invalid price range"
                        );
                    }

                    auto results =
                        searchService.filterByPrice(
                            minimum,
                            maximum
                        );

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", results.size()},
                            {"data", placesToJson(results)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Nearest places
        server.Get(
            "/api/nearest",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    double latitude =
                        getDoubleParam(request, "lat");

                    double longitude =
                        getDoubleParam(request, "lon");

                    int k = getIntParam(request, "k");

                    if (k < 1 || k > 100) {
                        throw std::invalid_argument(
                            "k must be between 1 and 100"
                        );
                    }

                    auto results =
                        searchService.findNearest(
                            latitude,
                            longitude,
                            static_cast<size_t>(k)
                        );

                    json data = json::array();

                    for (const auto& result : results) {

                        data.push_back({
                            {"place", placeToJson(result.first)},
                            {"distanceKm", result.second}
                        });
                    }

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", data.size()},
                            {"data", data}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        
        // Smart place recommendations
        server.Get(
            "/api/recommend",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    double latitude =
                        getDoubleParam(request, "lat");

                    double longitude =
                        getDoubleParam(request, "lon");

                    double radiusKm =
                        getDoubleParam(request, "radius");

                    std::string category =
                        request.has_param("category")
                        ? request.get_param_value("category")
                        : "";

                    int limit =
                        request.has_param("limit")
                        ? getIntParam(request, "limit")
                        : 5;

                    geosearch::RecommendationWeights weights;

                    if (request.has_param("distanceWeight")) {
                        weights.distance =
                            getDoubleParam(request, "distanceWeight");
                    }

                    if (request.has_param("ratingWeight")) {
                        weights.rating =
                            getDoubleParam(request, "ratingWeight");
                    }

                    if (request.has_param("priceWeight")) {
                        weights.price =
                            getDoubleParam(request, "priceWeight");
                    }

                    auto results =
                        searchService.recommendPlaces(
                            latitude,
                            longitude,
                            category,
                            radiusKm,
                            static_cast<size_t>(limit),
                            weights
                        );

                    json data = json::array();

                    for (const auto& item : results) {

                        data.push_back({
                            {"place", placeToJson(item.place)},
                            {"distanceKm", item.distanceKm},
                            {"distanceScore", item.distanceScore},
                            {"ratingScore", item.ratingScore >= 0.0 ? json(item.ratingScore) : json(nullptr)},
                            {"priceScore", item.priceScore >= 0.0 ? json(item.priceScore) : json(nullptr)},
                            {"overallScore", item.overallScore}
                        });
                    }

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", data.size()},
                            {"data", data}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {

                    sendError(response, 400, e.what());
                }
            }
        );


        // Top-rated places
        server.Get(
            "/api/top-rated",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    int k = getIntParam(request, "k");

                    if (k < 1 || k > 100) {
                        throw std::invalid_argument(
                            "k must be between 1 and 100"
                        );
                    }

                    auto results =
                        searchService.findTopRated(
                            static_cast<size_t>(k)
                        );

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", results.size()},
                            {"data", placesToJson(results)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Geographic map-window search
        server.Get(
            "/api/map-window",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    double minLat =
                        getDoubleParam(request, "minLat");

                    double maxLat =
                        getDoubleParam(request, "maxLat");

                    double minLon =
                        getDoubleParam(request, "minLon");

                    double maxLon =
                        getDoubleParam(request, "maxLon");

                    double minRating =
                        request.has_param("minRating")
                        ? getDoubleParam(request, "minRating")
                        : 0.0;

                    if (minLat < -90 || maxLat > 90 ||
                        minLon < -180 || maxLon > 180 ||
                        minRating < 0 || minRating > 5) {

                        throw std::invalid_argument(
                            "Invalid geographic search parameters"
                        );
                    }

                    auto results =
                        searchService.searchMapWindow(
                            minLat,
                            maxLat,
                            minLon,
                            maxLon,
                            minRating
                        );

                    response.set_content(
                        json({
                            {"success", true},
                            {"count", results.size()},
                            {"data", placesToJson(results)}
                        }).dump(4),
                        "application/json"
                    );

                } catch (const std::exception& e) {
                    sendError(response, 400, e.what());
                }
            }
        );

        // Serve browser frontend from web/
        server.set_mount_point("/", "./web");

        std::cout << "\nAvailable API endpoints:\n";
        std::cout << "GET /api/health\n";
        std::cout << "GET /api/places\n";
        std::cout << "GET /api/place?id=1\n";
        std::cout << "GET /api/search?keyword=medical\n";
        std::cout << "GET /api/category?name=Hospital\n";
        std::cout << "GET /api/rating?min=4&max=5\n";
        std::cout << "GET /api/price?min=100&max=500\n";
        std::cout << "GET /api/nearest?lat=28.6139&lon=77.209&k=3\n";
        std::cout << "GET /api/recommend?lat=28.6139&lon=77.209&radius=5&category=Hospital&limit=5\n";
        std::cout << "GET /api/top-rated?k=5\n";
        std::cout << "GET /api/map-window?minLat=28.61&maxLat=28.62&minLon=77.20&maxLon=77.22\n";

        std::cout << "\nServer URL: http://localhost:8080\n";
        std::cout << "Press Ctrl+C to stop the server.\n";

        if (!server.listen("0.0.0.0", 8080)) {
            throw std::runtime_error(
                "Unable to start server on port 8080"
            );
        }

    } catch (const std::exception& e) {

        std::cerr << "\nSERVER ERROR: "
                  << e.what() << '\n';

        return 1;
    }

    return 0;
}

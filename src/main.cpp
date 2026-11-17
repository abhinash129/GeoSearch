
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cctype>
#include <cstdio>
#include <nlohmann/json.hpp>

#include "geosearch/Place.h"
#include "geosearch/CSVLoader.h"
#include "geosearch/NearestPlaceSearch.h"

using namespace std;
using namespace geosearch;


// GeoSearch terminal dashboard colors
const string RESET  = "\033[0m";
const string BLUE   = "\033[1;34m";
const string CYAN   = "\033[1;36m";
const string GREEN  = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string WHITE  = "\033[1;37m";
const string GRAY   = "\033[0;37m";


void printHeader() {
    cout << "\n";
    cout << BLUE;
    cout << "==================================================\n";
    cout << "                  GeoSearch\n";
    cout << "          SMART LOCATION DISCOVERY ENGINE\n";
    cout << "==================================================\n";
    cout << RESET;

    cout << GREEN << "                 SYSTEM READY" << RESET << "\n\n";
}


void printSection(const string& title) {
    cout << "\n" << CYAN;
    cout << "==================================================\n";
    cout << "  " << title << "\n";
    cout << "==================================================\n";
    cout << RESET;
}


string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) {
            return static_cast<char>(tolower(c));
        });

    return text;
}


// Detect approximate location using IP address.
bool detectLocation(
    double& latitude,
    double& longitude,
    string& locationName
) {
    FILE* pipe = popen(
        "curl -fsS --max-time 8 https://ipwho.is/",
        "r"
    );

    if (!pipe)
        return false;

    char buffer[4096];
    string response;

    while (fgets(buffer, sizeof(buffer), pipe))
        response += buffer;

    int status = pclose(pipe);

    if (status != 0 || response.empty())
        return false;

    try {
        auto data = nlohmann::json::parse(response);

        if (!data.value("success", false))
            return false;

        double lat = data.at("latitude").get<double>();
        double lon = data.at("longitude").get<double>();

        if (lat < -90 || lat > 90 ||
            lon < -180 || lon > 180)
            return false;

        latitude = lat;
        longitude = lon;

        string city = data.value("city", "");
        string region = data.value("region", "");
        string country = data.value("country", "");

        locationName = city;

        if (!region.empty() && region != city)
            locationName += ", " + region;

        if (!country.empty())
            locationName += ", " + country;

        return true;

    } catch (...) {
        return false;
    }
}


// Get actual road distance and estimated driving time using OSRM.
bool getRoadRoute(
    double startLat,
    double startLon,
    double endLat,
    double endLon,
    double& distanceKm,
    double& durationMinutes
) {
    string url =
        "https://router.project-osrm.org/route/v1/driving/" +
        to_string(startLon) + "," + to_string(startLat) + ";" +
        to_string(endLon) + "," + to_string(endLat) +
        "?overview=false";

    string command =
        "curl -fsS --max-time 10 \"" + url + "\"";

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe)
        return false;

    char buffer[4096];
    string response;

    while (fgets(buffer, sizeof(buffer), pipe))
        response += buffer;

    int status = pclose(pipe);

    if (status != 0 || response.empty())
        return false;

    try {
        auto data = nlohmann::json::parse(response);

        if (data.value("code", "") != "Ok")
            return false;

        if (!data.contains("routes") ||
            data["routes"].empty())
            return false;

        double distanceMeters =
            data["routes"][0]["distance"].get<double>();

        double durationSeconds =
            data["routes"][0]["duration"].get<double>();

        distanceKm = distanceMeters / 1000.0;
        durationMinutes = durationSeconds / 60.0;

        return true;

    } catch (...) {
        return false;
    }
}


// Display individual place information.
void displayPlace(
    const Place& place,
    double distance,
    int position
) {
    cout << "\n" << WHITE << position << ". "
         << place.name << RESET;

    if (position == 1)
        cout << "  " << GREEN << "[NEAREST]" << RESET;

    cout << "\n";

    cout << "   " << CYAN << "Category : " << RESET
         << place.category << "\n";

    cout << "   " << CYAN << "Address  : " << RESET
         << (place.address.empty() ? "Not available" : place.address)
         << "\n";

    cout << fixed << setprecision(2);

    cout << "   " << YELLOW << "Distance : " << RESET
         << distance << " km (straight-line)\n";

    cout << "   Rating   : ";

    if (place.hasRating()) {
        cout << place.rating << "/5";

        if (place.id == 188)
            cout << " (WIB Open Data - Editorial Rating)";

        cout << "\n";
    } else {
        cout << "Not available\n";
    }

    cout << "   Price    : ";

    if (place.hasPrice())
        cout << place.price << "\n";
    else
        cout << "Not available\n";

    if (!place.openingHours.empty())
        cout << "   Hours    : " << place.openingHours << "\n";

    if (!place.phone.empty())
        cout << "   Phone    : " << place.phone << "\n";

    if (!place.website.empty())
        cout << "   Website  : " << place.website << "\n";

    if (!place.cuisine.empty())
        cout << "   Cuisine  : " << place.cuisine << "\n";
}


int main() {
    try {

        printHeader();

        vector<Place> places =
            CSVLoader::load("data/places.csv");

        if (places.empty()) {
            cout << "No places found in the dataset.\n";
            return 1;
        }

        // Default fallback location: Noida Sector 62.
        double latitude = 28.62115;
        double longitude = 77.36435;

        string locationName =
            "Noida, Uttar Pradesh, India";

        printSection("YOUR CURRENT LOCATION");

        cout << YELLOW
             << "  Detecting approximate location using IP..."
             << RESET << "\n";

        if (detectLocation(latitude, longitude, locationName)) {

            cout << "\n  " << GREEN
                 << "Location detected successfully"
                 << RESET << "\n";

            cout << "  " << WHITE
                 << locationName << RESET << "\n";

            cout << GRAY
                 << "  Latitude  : " << fixed << setprecision(5)
                 << latitude << "\n";

            cout << "  Longitude : "
                 << longitude << RESET << "\n";

            cout << "\nUse this location? (Y/N): ";

            string choice;
            getline(cin, choice);

            if (toLower(choice) == "n") {

                cout << "\nEnter your latitude: ";

                if (!(cin >> latitude)) {
                    cout << "Invalid latitude.\n";
                    return 1;
                }

                cout << "Enter your longitude: ";

                if (!(cin >> longitude)) {
                    cout << "Invalid longitude.\n";
                    return 1;
                }

                cin.ignore(
                    numeric_limits<streamsize>::max(), '\n'
                );

                locationName = "Manual coordinates";
            }

        } else {

            cout << "\nAutomatic location detection failed.\n";
            cout << "Using demo location: "
                 << locationName << "\n";

            cout << "\nUse demo location? (Y/N): ";

            string choice;
            getline(cin, choice);

            if (toLower(choice) == "n") {

                cout << "Enter your latitude: ";

                if (!(cin >> latitude)) {
                    cout << "Invalid latitude.\n";
                    return 1;
                }

                cout << "Enter your longitude: ";

                if (!(cin >> longitude)) {
                    cout << "Invalid longitude.\n";
                    return 1;
                }

                cin.ignore(
                    numeric_limits<streamsize>::max(), '\n'
                );

                locationName = "Manual coordinates";
            }
        }

        if (latitude < -90 || latitude > 90 ||
            longitude < -180 || longitude > 180) {

            cout << "Invalid coordinates.\n";
            return 1;
        }

        printSection("SELECTED LOCATION");

        cout << "  " << GREEN
             << locationName << RESET << "\n";

        cout << GRAY
             << "  Latitude  : " << fixed << setprecision(5)
             << latitude << "\n";

        cout << "  Longitude : "
             << longitude << RESET << "\n";

        cout << "\nPlaces loaded: "
             << places.size() << "\n";

        cout << "\n==================================================\n";


        // Continuous search loop.
        while (true) {

            printSection("WHAT WOULD YOU LIKE TO FIND?");

            cout << GRAY
                 << "  Examples: hospital | cafe | restaurant | pharmacy | bookstore\n\n"
                 << RESET;

            cout << "  " << WHITE
                 << "Search: " << RESET;

            string query;
            getline(cin, query);

            if (toLower(query) == "exit" ||
                toLower(query) == "quit") {

                cout << "\nThank you for using GeoSearch!\n";
                break;
            }

            if (query.empty())
                continue;

            string keyword = toLower(query);


            // Remove common search phrases.
            vector<string> prefixes = {
                "nearest ", "nearby ", "find ", "search "
            };

            for (const string& prefix : prefixes) {

                if (keyword.rfind(prefix, 0) == 0) {
                    keyword = keyword.substr(prefix.size());
                    break;
                }
            }


            // Basic category aliases.
            if (keyword == "restaurants")
                keyword = "restaurant";

            else if (keyword == "cafes" ||
                     keyword == "coffee shop")
                keyword = "cafe";

            else if (keyword == "pharmacies")
                keyword = "pharmacy";

            else if (keyword == "book shop" ||
                     keyword == "book shops")
                keyword = "bookstore";


            // Search and category filtering.
            vector<Place> matches;

            for (const Place& place : places) {

                string category = toLower(place.category);
                string name = toLower(place.name);
                string description = toLower(place.description);

                // Identify searches for supported categories.
                bool categorySearch =
                    keyword == "hospital" ||
                    keyword == "cafe" ||
                    keyword == "restaurant" ||
                    keyword == "pharmacy" ||
                    keyword == "bookstore";

                bool isMatch = false;

                if (categorySearch) {

                    // Strict category matching.
                    isMatch =
                        category.find(keyword) != string::npos;

                    // Include places explicitly named as cafes.
                    if (keyword == "cafe" &&
                        name.find("cafe") != string::npos) {

                        isMatch = true;
                    }

                } else {

                    // General keyword search.
                    isMatch =
                        category.find(keyword) != string::npos ||
                        name.find(keyword) != string::npos ||
                        description.find(keyword) != string::npos;
                }

                if (isMatch) {
                    matches.push_back(place);
                }
            }


            printSection("SEARCH RESULTS: " + query);

            if (matches.empty()) {

                cout << "\nNo matching places found in the current dataset.\n";

                cout << "Try: hospital, cafe, restaurant, pharmacy, ATM, school.\n";

                continue;
            }


            // Find nearest places using geographic distance.
            NearestPlaceSearch nearestSearch;

            nearestSearch.build(matches);

            size_t resultLimit =
                min(size_t(10), matches.size());

            auto results = nearestSearch.nearest(
                latitude,
                longitude,
                resultLimit
            );


            cout << "\nMatching places: "
                 << matches.size() << "\n";

            cout << "Showing nearest "
                 << results.size() << " places.\n";


            for (size_t i = 0; i < results.size(); ++i) {

                displayPlace(
                    results[i].first,
                    results[i].second,
                    static_cast<int>(i + 1)
                );
            }


            // Get road route to the nearest geographic result.
            if (!results.empty()) {

                const Place& nearestPlace =
                    results[0].first;

                cout << "\nNEAREST MATCHING PLACE\n";

                cout << nearestPlace.name
                     << " is the nearest matching place, at "
                     << fixed << setprecision(2)
                     << results[0].second
                     << " km (straight-line).\n";


                double roadDistance = 0.0;
                double drivingTime = 0.0;

                cout << "\nCalculating road route...\n";


                if (getRoadRoute(
                        latitude,
                        longitude,
                        nearestPlace.latitude,
                        nearestPlace.longitude,
                        roadDistance,
                        drivingTime
                    )) {

                    printSection("ROAD ROUTE INFORMATION");

                    cout << "   " << CYAN
                         << "Road distance : " << RESET
                         << GREEN << roadDistance
                         << " km" << RESET << "\n";

                    cout << "   " << CYAN
                         << "Driving time  : " << RESET
                         << GREEN << drivingTime
                         << " minutes" << RESET << "\n";

                } else {

                    cout << "\nRoad route currently unavailable.\n";
                }
            }


            cout << "\nNote: The displayed place rankings use "
                 << "straight-line distance. Road routing is "
                 << "calculated for the nearest result only.\n";

            cout << "==================================================\n";
        }

    } catch (const exception& error) {

        cerr << "\nGeoSearch error: "
             << error.what() << "\n";

        return 1;
    }

    return 0;
}

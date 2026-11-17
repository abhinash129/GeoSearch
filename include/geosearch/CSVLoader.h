
#pragma once

#include "Place.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace geosearch {

class CSVLoader {

private:

    static std::vector<std::string> parseLine(
        const std::string& line
    ) {
        std::vector<std::string> fields;
        std::string field;
        bool insideQuotes = false;

        for (size_t i = 0; i < line.size(); ++i) {

            char ch = line[i];

            if (ch == '"') {

                if (insideQuotes &&
                    i + 1 < line.size() &&
                    line[i + 1] == '"') {

                    field += '"';
                    ++i;

                } else {
                    insideQuotes = !insideQuotes;
                }

            } else if (ch == ',' && !insideQuotes) {

                fields.push_back(field);
                field.clear();

            } else {
                field += ch;
            }
        }

        fields.push_back(field);

        return fields;
    }

public:

    static std::vector<Place> load(
        const std::string& filename
    ) {

        std::ifstream file(filename);

        if (!file.is_open()) {
            throw std::runtime_error(
                "Unable to open CSV file: " + filename
            );
        }

        std::vector<Place> places;
        std::string line;

        if (!std::getline(file, line)) {
            return places;
        }

        size_t invalidRecords = 0;

        while (std::getline(file, line)) {

            // Handle Windows CRLF line endings.
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) {
                continue;
            }

            try {

                std::vector<std::string> fields =
                    parseLine(line);

                // Support both legacy (8-column) and extended
                // (15-column) CSV files.
                if (fields.size() != 8 &&
                    fields.size() != 15 &&
                    fields.size() != 17) {
                    ++invalidRecords;
                    continue;
                }

                Place place;

                place.id = std::stoi(fields[0]);
                place.name = fields[1];
                place.category = fields[2];
                place.description = fields[3];

                place.latitude = std::stod(fields[4]);
                place.longitude = std::stod(fields[5]);
                
                place.rating = fields[6].empty()
                    ? -1.0
                    : std::stod(fields[6]);

                place.price = fields[7].empty()
                    ? -1.0
                    : std::stod(fields[7]);

                // The additional fields exist only in the extended format.
                if (fields.size() == 15) {
                    place.cuisine = fields[8];
                    place.openingHours = fields[9];
                    place.phone = fields[10];
                    place.website = fields[11];
                    place.brand = fields[12];
                    place.operatorName = fields[13];
                    place.address = fields[14];
                }

                // Stable OpenStreetMap identity (17-column format).
                if (fields.size() == 17) {
                    place.osmType = fields[15];
                    place.osmId = fields[16];
                }

                if (!place.isValid()) {
                    ++invalidRecords;
                    continue;
                }

                places.push_back(place);

            } catch (const std::exception& e) {

                std::cerr << "CSV parsing error: " << e.what()
                          << " | Row: " << line << '\n';

                ++invalidRecords;
            }
        }

        std::cout << "CSV Loading Complete\n";

        std::cout << "Valid records: "
                  << places.size() << '\n';

        std::cout << "Invalid records: "
                  << invalidRecords << '\n';

        return places;
    }
};

} // namespace geosearch

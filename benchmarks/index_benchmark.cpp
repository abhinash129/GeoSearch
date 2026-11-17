#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "geosearch/Index.h"
#include "geosearch/Place.h"
#include "geosearch/BSTIndex.h"
#include "geosearch/AVLIndex.h"
#include "geosearch/RedBlackTreeIndex.h"
#include "geosearch/SplayTreeIndex.h"
#include "geosearch/BTreeIndex.h"
#include "geosearch/HashChainingIndex.h"
#include "geosearch/OpenAddressingHashIndex.h"

using namespace geosearch;
using Clock = std::chrono::steady_clock;

struct BenchmarkResult {
    std::string indexName;
    size_t records = 0;
    double insertionMs = 0;
    double successfulSearchMs = 0;
    double unsuccessfulSearchMs = 0;
    double deletionMs = 0;
    bool correct = false;
};

Place makePlace(int id) {
    Place place;

    place.id = id;
    place.name = "Place " + std::to_string(id);
    place.category = "Test";
    place.description = "Generated benchmark place";
    place.latitude = 20.0 + (id % 60) * 0.01;
    place.longitude = 70.0 + (id % 100) * 0.01;
    place.rating = 1.0 + (id % 41) * 0.1;
    place.price = 100.0 + (id % 1000);

    return place;
}

double elapsedMilliseconds(
    Clock::time_point start,
    Clock::time_point finish
) {
    return std::chrono::duration<double, std::milli>(
        finish - start
    ).count();
}

using IndexFactory = std::function<std::unique_ptr<Index>()>;

std::vector<std::pair<std::string, IndexFactory>>
createIndexFactories() {
    return {
        {
            "BST",
            [] {
                return std::make_unique<BSTIndex>();
            }
        },
        {
            "AVL",
            [] {
                return std::make_unique<AVLIndex>();
            }
        },
        {
            "Red-Black Tree",
            [] {
                return std::make_unique<RedBlackTreeIndex>();
            }
        },
        {
            "Splay Tree",
            [] {
                return std::make_unique<SplayTreeIndex>();
            }
        },
        {
            "B-Tree",
            [] {
                return std::make_unique<BTreeIndex>();
            }
        },
        {
            "Separate Chaining",
            [] {
                return std::make_unique<HashChainingIndex>();
            }
        },
        {
            "Linear Probing",
            [] {
                return std::make_unique<OpenAddressingHashIndex>(
                    ProbingStrategy::LINEAR
                );
            }
        },
        {
            "Quadratic Probing",
            [] {
                return std::make_unique<OpenAddressingHashIndex>(
                    ProbingStrategy::QUADRATIC
                );
            }
        },
        {
            "Double Hashing",
            [] {
                return std::make_unique<OpenAddressingHashIndex>(
                    ProbingStrategy::DOUBLE_HASHING
                );
            }
        }
    };
}

BenchmarkResult benchmarkIndex(
    const std::string& indexName,
    const IndexFactory& factory,
    const std::vector<Place>& places
) {
    BenchmarkResult result;
    result.indexName = indexName;
    result.records = places.size();

    std::unique_ptr<Index> index = factory();

    bool correct = true;

    // 1. Insertion benchmark
    auto start = Clock::now();

    for (const auto& place : places) {
        if (!index->insert(place)) {
            correct = false;
        }
    }

    auto finish = Clock::now();

    result.insertionMs =
        elapsedMilliseconds(start, finish);

    if (index->size() != places.size()) {
        correct = false;
    }

    // 2. Successful search benchmark
    start = Clock::now();

    for (const auto& place : places) {
        Place* found = index->search(place.id);

        if (found == nullptr || found->id != place.id) {
            correct = false;
        }
    }

    finish = Clock::now();

    result.successfulSearchMs =
        elapsedMilliseconds(start, finish);

    // 3. Unsuccessful search benchmark
    start = Clock::now();

    for (size_t i = 1; i <= places.size(); ++i) {
        int missingId =
            static_cast<int>(places.size() + i);

        if (index->search(missingId) != nullptr) {
            correct = false;
        }
    }

    finish = Clock::now();

    result.unsuccessfulSearchMs =
        elapsedMilliseconds(start, finish);

    // 4. Deletion benchmark
    start = Clock::now();

    size_t deletedCount = 0;

    for (size_t i = 0; i < places.size(); i += 2) {
        if (index->remove(places[i].id)) {
            ++deletedCount;
        } else {
            correct = false;
        }
    }

    finish = Clock::now();

    result.deletionMs =
        elapsedMilliseconds(start, finish);

    // 5. Verify deletion and remaining records
    size_t expectedDeleted = (places.size() + 1) / 2;

    if (deletedCount != expectedDeleted) {
        correct = false;
    }

    if (index->size() != places.size() - expectedDeleted) {
        correct = false;
    }

    for (size_t i = 0; i < places.size(); ++i) {
        bool shouldExist = (i % 2 != 0);

        Place* found = index->search(places[i].id);

        if (shouldExist && found == nullptr) {
            correct = false;
        }

        if (!shouldExist && found != nullptr) {
            correct = false;
        }
    }

    result.correct = correct;

    return result;
}

int main(int argc, char* argv[]) {
    std::vector<size_t> datasetSizes = {
        100,
        1000,
        10000,
        100000
    };

    if (argc > 1) {
        datasetSizes.clear();

        for (int i = 1; i < argc; ++i) {
            try {
                size_t size = std::stoull(argv[i]);

                if (size == 0) {
                    throw std::invalid_argument("Zero size");
                }

                datasetSizes.push_back(size);
            } catch (...) {
                std::cerr
                    << "Invalid dataset size: "
                    << argv[i] << '\n';

                return 1;
            }
        }
    }

    std::filesystem::create_directories("benchmarks");

    std::ofstream csv("benchmarks/results.csv");

    if (!csv.is_open()) {
        std::cerr << "Could not create benchmarks/results.csv\n";
        return 1;
    }

    csv << "Index,Records,Insertion_ms,"
        << "Successful_Search_ms,"
        << "Unsuccessful_Search_ms,"
        << "Deletion_ms,Correctness\n";

    std::cout
        << "=============================================\n"
        << "          GEOSEARCH INDEX BENCHMARK\n"
        << "=============================================\n";

    auto factories = createIndexFactories();

    bool allCorrect = true;

    for (size_t datasetSize : datasetSizes) {
        std::cout << "\n=============================================\n";
        std::cout << "DATASET SIZE: " << datasetSize << '\n';
        std::cout << "=============================================\n";

        std::vector<Place> places;
        places.reserve(datasetSize);

        for (size_t i = 1; i <= datasetSize; ++i) {
            places.push_back(makePlace(static_cast<int>(i)));
        }

        // Shuffle insertion order to avoid sorted-input bias.
        std::mt19937 generator(42);

        std::shuffle(
            places.begin(),
            places.end(),
            generator
        );

        for (const auto& [name, factory] : factories) {
            BenchmarkResult result =
                benchmarkIndex(name, factory, places);

            std::cout << "\nIndex: " << result.indexName << '\n';
            std::cout << "Records: " << result.records << '\n';

            std::cout << std::fixed << std::setprecision(3);

            std::cout << "Insertion: "
                      << result.insertionMs << " ms\n";

            std::cout << "Successful search: "
                      << result.successfulSearchMs << " ms\n";

            std::cout << "Unsuccessful search: "
                      << result.unsuccessfulSearchMs << " ms\n";

            std::cout << "Deletion: "
                      << result.deletionMs << " ms\n";

            std::cout << "Correctness: "
                      << (result.correct ? "PASSED" : "FAILED")
                      << '\n';

            csv << result.indexName << ','
                << result.records << ','
                << result.insertionMs << ','
                << result.successfulSearchMs << ','
                << result.unsuccessfulSearchMs << ','
                << result.deletionMs << ','
                << (result.correct ? "PASSED" : "FAILED")
                << '\n';

            if (!result.correct) {
                allCorrect = false;
            }
        }
    }

    csv.close();

    std::cout << "\n=============================================\n";
    std::cout << "BENCHMARK COMPLETED\n";
    std::cout << "Results: benchmarks/results.csv\n";
    std::cout << "Overall correctness: "
              << (allCorrect ? "PASSED" : "FAILED")
              << '\n';
    std::cout << "=============================================\n";

    return allCorrect ? 0 : 1;
}
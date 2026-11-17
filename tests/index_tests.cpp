#include <iostream>
#include <memory>
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

Place makePlace(int id) {
    Place place;

    place.id = id;
    place.name = "Test Place " + std::to_string(id);
    place.category = "Test";
    place.description = "Unit test record";
    place.latitude = 28.0;
    place.longitude = 77.0;
    place.rating = 4.5;
    place.price = 200.0;

    return place;
}

void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testIndex(Index& index) {
    std::cout << "Testing: " << index.name() << '\n';

    Place p1 = makePlace(101);
    Place p2 = makePlace(202);
    Place p3 = makePlace(303);
    Place duplicate = makePlace(101);

    check(index.size() == 0, "Initial size should be zero");

    check(index.insert(p1), "First insertion failed");
    check(index.insert(p2), "Second insertion failed");
    check(index.insert(p3), "Third insertion failed");

    check(index.size() == 3, "Size after insertion should be 3");

    check(!index.insert(duplicate), "Duplicate insertion was accepted");

    Place* found = index.search(202);

    check(found != nullptr, "Search failed for existing ID");
    check(found->name == p2.name, "Incorrect search result");

    check(index.search(999) == nullptr,
          "Non-existing ID should not be found");

    check(index.remove(202), "Deletion failed");

    check(index.search(202) == nullptr,
          "Deleted record was still found");

    check(index.size() == 2,
          "Size after deletion should be 2");

    check(!index.remove(999),
          "Deletion of non-existing record should fail");

    check(index.search(101) != nullptr,
          "First record disappeared unexpectedly");

    check(index.search(303) != nullptr,
          "Third record disappeared unexpectedly");

    std::cout << "  PASSED\n\n";
}

int main() {
    std::cout
        << "====================================\n"
        << "       GEOSEARCH INDEX TESTS\n"
        << "====================================\n\n";

    try {
        std::vector<std::unique_ptr<Index>> indexes;

        indexes.push_back(std::make_unique<BSTIndex>());
        indexes.push_back(std::make_unique<AVLIndex>());
        indexes.push_back(std::make_unique<RedBlackTreeIndex>());
        indexes.push_back(std::make_unique<SplayTreeIndex>());
        indexes.push_back(std::make_unique<BTreeIndex>());
        indexes.push_back(std::make_unique<HashChainingIndex>());

        indexes.push_back(
            std::make_unique<OpenAddressingHashIndex>(
                ProbingStrategy::LINEAR
            )
        );

        indexes.push_back(
            std::make_unique<OpenAddressingHashIndex>(
                ProbingStrategy::QUADRATIC
            )
        );

        indexes.push_back(
            std::make_unique<OpenAddressingHashIndex>(
                ProbingStrategy::DOUBLE_HASHING
            )
        );

        for (auto& index : indexes) {
            testIndex(*index);
        }

        std::cout
            << "====================================\n"
            << " ALL INDEX TESTS PASSED\n"
            << "====================================\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "\nTEST FAILED: "
                  << error.what() << '\n';

        return 1;
    }
}
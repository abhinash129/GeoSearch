
#pragma once

#include "Place.h"

#include <cstddef>
#include <string>

namespace geosearch {

class Index {
public:
    virtual ~Index() = default;

    virtual bool insert(const Place& place) = 0;

    virtual Place* search(int id) = 0;

    virtual bool remove(int id) = 0;

    virtual size_t size() const = 0;

    virtual std::string name() const = 0;
};

} // namespace geosearch

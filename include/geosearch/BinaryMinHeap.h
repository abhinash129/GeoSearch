
#pragma once

#include "Place.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace geosearch {

class BinaryMinHeap {

private:

    struct HeapEntry {
        Place place;
        double priority;
    };

    std::vector<HeapEntry> heap;

    void swapEntries(size_t a, size_t b) {
        std::swap(heap[a], heap[b]);
    }

    void heapifyUp(size_t index) {

        while (index > 0) {

            size_t parent = (index - 1) / 2;

            if (heap[parent].priority <= heap[index].priority)
                break;

            swapEntries(parent, index);
            index = parent;
        }
    }

    void heapifyDown(size_t index) {

        while (true) {

            size_t smallest = index;
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;

            if (left < heap.size() &&
                heap[left].priority < heap[smallest].priority) {
                smallest = left;
            }

            if (right < heap.size() &&
                heap[right].priority < heap[smallest].priority) {
                smallest = right;
            }

            if (smallest == index)
                break;

            swapEntries(index, smallest);
            index = smallest;
        }
    }

    size_t findIndex(int id) const {

        for (size_t i = 0; i < heap.size(); ++i) {
            if (heap[i].place.id == id)
                return i;
        }

        return heap.size();
    }

public:

    bool insert(const Place& place, double priority) {

        if (!place.isValid() ||
            !std::isfinite(priority) ||
            findIndex(place.id) != heap.size()) {
            return false;
        }

        heap.push_back({place, priority});

        heapifyUp(heap.size() - 1);

        return true;
    }

    const Place* peek() const {

        if (heap.empty())
            return nullptr;

        return &heap[0].place;
    }

    bool extractMin(Place& result) {

        if (heap.empty())
            return false;

        result = heap[0].place;

        if (heap.size() == 1) {
            heap.pop_back();
            return true;
        }

        heap[0] = heap.back();
        heap.pop_back();

        heapifyDown(0);

        return true;
    }

    const Place* search(int id) const {

        size_t index = findIndex(id);

        if (index == heap.size())
            return nullptr;

        return &heap[index].place;
    }

    bool remove(int id) {

        size_t index = findIndex(id);

        if (index == heap.size())
            return false;

        if (index == heap.size() - 1) {
            heap.pop_back();
            return true;
        }

        heap[index] = heap.back();
        heap.pop_back();

        if (index > 0 &&
            heap[index].priority <
            heap[(index - 1) / 2].priority) {

            heapifyUp(index);

        } else {

            heapifyDown(index);
        }

        return true;
    }

    bool updatePriority(int id, double newPriority) {

        if (!std::isfinite(newPriority))
            return false;

        size_t index = findIndex(id);

        if (index == heap.size())
            return false;

        double oldPriority = heap[index].priority;

        heap[index].priority = newPriority;

        if (newPriority < oldPriority)
            heapifyUp(index);
        else
            heapifyDown(index);

        return true;
    }

    size_t size() const {
        return heap.size();
    }

    bool empty() const {
        return heap.empty();
    }

    void display() const {

        for (const auto& entry : heap) {

            std::cout << "ID: " << entry.place.id
                      << " | Name: " << entry.place.name
                      << " | Priority: " << entry.priority
                      << '\n';
        }
    }
};

} // namespace geosearch

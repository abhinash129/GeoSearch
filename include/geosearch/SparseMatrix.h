
#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace geosearch {

class SparseMatrix {

private:

    struct Entry {
        int row;
        int col;
        int value;
    };

    int rows;
    int cols;

    std::vector<Entry> entries;

public:

    SparseMatrix(int r, int c) : rows(r), cols(c) {

        if (r <= 0 || c <= 0) {
            throw std::invalid_argument(
                "Matrix dimensions must be positive"
            );
        }
    }

    void set(int row, int col, int value) {

        if (row < 0 || row >= rows ||
            col < 0 || col >= cols) {
            throw std::out_of_range("Invalid matrix position");
        }

        for (auto it = entries.begin(); it != entries.end(); ++it) {

            if (it->row == row && it->col == col) {

                if (value == 0) {
                    entries.erase(it);
                } else {
                    it->value = value;
                }

                return;
            }
        }

        if (value != 0) {
            entries.push_back({row, col, value});
        }
    }

    int get(int row, int col) const {

        if (row < 0 || row >= rows ||
            col < 0 || col >= cols) {
            throw std::out_of_range("Invalid matrix position");
        }

        for (const auto& entry : entries) {

            if (entry.row == row && entry.col == col) {
                return entry.value;
            }
        }

        return 0;
    }

    int nonZeroCount() const {
        return static_cast<int>(entries.size());
    }

    int getRows() const {
        return rows;
    }

    int getCols() const {
        return cols;
    }

    void displayTriplets() const {

        std::cout << "Row  Column  Value\n";

        for (const auto& entry : entries) {

            std::cout << entry.row << "     "
                      << entry.col << "       "
                      << entry.value << '\n';
        }
    }

    void display() const {

        for (int i = 0; i < rows; ++i) {

            for (int j = 0; j < cols; ++j) {
                std::cout << get(i, j) << "\t";
            }

            std::cout << '\n';
        }
    }

    SparseMatrix transpose() const {

        SparseMatrix result(cols, rows);

        for (const auto& entry : entries) {
            result.set(entry.col, entry.row, entry.value);
        }

        return result;
    }
};

} // namespace geosearch

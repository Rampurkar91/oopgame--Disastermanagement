#pragma once
#include <string>
#include <iostream>

namespace drc {

struct Position {
    int row = 0;
    int col = 0;
    Position() = default;
    Position(int r, int c) : row(r), col(c) {}
    bool operator==(const Position& o) const { return row == o.row && col == o.col; }
    bool operator!=(const Position& o) const { return !(*this == o); }
    bool operator<(const Position& o) const {
        if (row != o.row) return row < o.row;
        return col < o.col;
    }
    Position operator+(const Position& o) const { return {row + o.row, col + o.col}; }
    std::string toString() const { return "(" + std::to_string(row) + "," + std::to_string(col) + ")"; }
    friend std::ostream& operator<<(std::ostream& os, const Position& p) {
        os << p.toString();
        return os;
    }
};

} // namespace drc

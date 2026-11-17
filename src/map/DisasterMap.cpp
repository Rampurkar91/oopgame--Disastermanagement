#include "../../include/map/DisasterMap.hpp"
#include <algorithm>
#include <random>

namespace drc {

DisasterMap::DisasterMap() { loadClassicMap(); }

void DisasterMap::loadClassicMap() {
    grid_.assign(ROWS, std::vector<CellType>(COLS, CellType::Road));
    // Houses border
    for (int c = 0; c < COLS; ++c) { grid_[0][c] = CellType::House; grid_[ROWS-1][c] = CellType::House; }
    for (int r = 0; r < ROWS; ++r) { grid_[r][0] = CellType::House; grid_[r][COLS-1] = CellType::House; }
    // Facilities
    grid_[0][11] = CellType::Hospital;
    grid_[8][11] = CellType::Hospital;
    grid_[2][8] = CellType::FireStation;
    // Blocked roads
    std::vector<Position> blocked = {{2,3},{3,3},{5,5},{6,5},{4,7},{1,6}};
    for (auto p : blocked) grid_[p.row][p.col] = CellType::Blocked;
    // Flood
    std::vector<Position> flood = {{6,2},{6,3},{7,2},{7,3},{3,9},{4,9}};
    for (auto p : flood) grid_[p.row][p.col] = CellType::Flood;
    // Fire
    std::vector<Position> fire = {{2,5},{2,6},{5,8},{6,8}};
    for (auto p : fire) grid_[p.row][p.col] = CellType::Fire;
    // Keep facilities intact
    grid_[0][11] = CellType::Hospital; grid_[8][11] = CellType::Hospital; grid_[2][8] = CellType::FireStation;
}

void DisasterMap::loadLevel(int level) {
    loadClassicMap();
    if (level <= 1) return; // beginner: classic
    std::mt19937 rng(1000 + level);
    auto scatter = [&](CellType t, int n) {
        int placed = 0, guard = 0;
        while (placed < n && guard++ < 500) {
            int r = 1 + (rng() % (ROWS-2));
            int c = 1 + (rng() % (COLS-2));
            if (grid_[r][c] == CellType::Road) { grid_[r][c] = t; ++placed; }
        }
    };
    if (level == 2) { scatter(CellType::Flood, 4); scatter(CellType::Blocked, 4); }
    if (level == 3) { scatter(CellType::Flood, 6); scatter(CellType::Blocked, 6); scatter(CellType::Fire, 3); }
    if (level == 4) { scatter(CellType::Flood, 8); scatter(CellType::Blocked, 8); scatter(CellType::Fire, 5); }
    if (level == 5) { scatter(CellType::Flood, 10); scatter(CellType::Blocked, 10); scatter(CellType::Fire, 7); }
    grid_[0][11] = CellType::Hospital; grid_[8][11] = CellType::Hospital; grid_[2][8] = CellType::FireStation;
}

void DisasterMap::randomize(int level, unsigned seed) {
    loadLevel(level);
    if (seed == 0) return;
    std::mt19937 rng(seed);
    // re-scatter a few hazards deterministically
    for (int i = 0; i < 6; ++i) {
        int r = 1 + (rng() % (ROWS-2));
        int c = 1 + (rng() % (COLS-2));
        if (grid_[r][c] == CellType::Road) grid_[r][c] = (i % 3 == 0) ? CellType::Fire : (i % 3 == 1 ? CellType::Flood : CellType::Blocked);
    }
    grid_[0][11] = CellType::Hospital; grid_[8][11] = CellType::Hospital; grid_[2][8] = CellType::FireStation;
}

bool DisasterMap::inBounds(Position p) const { return p.row >= 0 && p.row < ROWS && p.col >= 0 && p.col < COLS; }
CellType DisasterMap::getCell(Position p) const { return inBounds(p) ? grid_[p.row][p.col] : CellType::Blocked; }
void DisasterMap::setCell(Position p, CellType t) { if (inBounds(p)) grid_[p.row][p.col] = t; }

bool DisasterMap::isWalkable(Position p) const {
    if (!inBounds(p)) return false;
    CellType c = grid_[p.row][p.col];
    // BUG FIX: FIRE is now impassable for ordinary ground vehicles.
    return c != CellType::Blocked && c != CellType::Flood && c != CellType::Fire;
}

bool DisasterMap::isWalkableFor(Position p, const std::string& vehicleKind) const {
    if (!inBounds(p)) return false;
    CellType c = grid_[p.row][p.col];
    if (vehicleKind == "Helicopter") return true; // aerial
    if (vehicleKind == "FireTruck") return c != CellType::Blocked && c != CellType::Flood; // may enter FIRE
    return isWalkable(p);
}

std::vector<Position> DisasterMap::neighbors(Position p) const {
    std::vector<Position> out;
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    for (int i = 0; i < 4; ++i) {
        Position n{p.row + dr[i], p.col + dc[i]};
        if (inBounds(n)) out.push_back(n);
    }
    return out;
}

} // namespace drc

#pragma once
#include "../core/Position.hpp"
#include <vector>
#include <string>

namespace drc {

enum class CellType { Road, Blocked, Flood, Fire, House, Hospital, FireStation };

inline std::string toString(CellType t) {
    switch (t) {
        case CellType::Road: return "ROAD";
        case CellType::Blocked: return "BLOCKED";
        case CellType::Flood: return "FLOOD";
        case CellType::Fire: return "FIRE";
        case CellType::House: return "HOUSE";
        case CellType::Hospital: return "HOSPITAL";
        case CellType::FireStation: return "FIRE_STATION";
    }
    return "ROAD";
}

class DisasterMap {
public:
    static constexpr int ROWS = 9;
    static constexpr int COLS = 13;
    DisasterMap();
    void loadClassicMap();
    void loadLevel(int level); // 1..5
    void randomize(int level, unsigned seed = 0);
    bool inBounds(Position p) const;
    CellType getCell(Position p) const;
    void setCell(Position p, CellType t);
    // Ground-vehicle walkability: BLOCKED, FLOOD, FIRE are impassable.
    bool isWalkable(Position p) const;
    // Vehicle-aware walkability (FireTruck may enter FIRE, Helicopter flies over all).
    bool isWalkableFor(Position p, const std::string& vehicleKind) const;
    std::vector<Position> neighbors(Position p) const;
    int rows() const { return ROWS; }
    int cols() const { return COLS; }
    const std::vector<std::vector<CellType>>& grid() const { return grid_; }
private:
    std::vector<std::vector<CellType>> grid_;
};

} // namespace drc

#include "../../include/algorithms/RouteStrategy.hpp"
#include <algorithm>
namespace drc {
int FastestRouteStrategy::edgeCost(Position, Position to, const DisasterMap& map) const {
    CellType c = map.getCell(to);
    if (c == CellType::Fire) return 3; // only FireTruck/Helicopter reach here
    if (c == CellType::House) return 2;
    return 1;
}
int SafestRouteStrategy::edgeCost(Position from, Position to, const DisasterMap& map) const {
    CellType c = map.getCell(to);
    int cost = 1;
    if (c == CellType::Fire) cost += 10;
    if (c == CellType::Flood) cost += 8;
    if (c == CellType::House) cost += 2;
    // Penalize cells adjacent to hazards (risk proximity).
    for (auto n : map.neighbors(to)) {
        CellType nc = map.getCell(n);
        if (nc == CellType::Fire || nc == CellType::Flood) cost += 3;
        if (nc == CellType::Blocked) cost += 1;
    }
    (void)from;
    return cost;
}
int LowFuelRouteStrategy::edgeCost(Position, Position to, const DisasterMap& map) const {
    CellType c = map.getCell(to);
    // Prefer roads (cost 1), avoid houses (cost 3), fire is expensive.
    if (c == CellType::Road || c == CellType::Hospital || c == CellType::FireStation) return 1;
    if (c == CellType::House) return 3;
    if (c == CellType::Fire) return 5;
    return 2;
}
} // namespace drc

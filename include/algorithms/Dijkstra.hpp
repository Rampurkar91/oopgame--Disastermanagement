#pragma once
#include "../core/Position.hpp"
#include "../map/DisasterMap.hpp"
#include "../core/GameState.hpp"
#include "RouteStrategy.hpp"
#include <vector>
#include <memory>

namespace drc {

struct DijkstraResult {
    bool found = false;
    std::vector<Position> path;          // start..goal inclusive
    int distance = -1;                   // total weighted cost
    int steps = 0;                       // number of moves (path.size()-1)
    std::vector<Position> visitedOrder;  // order nodes were finalized
    std::vector<std::vector<int>> dist;  // distance array
    std::vector<std::vector<Position>> pred; // predecessor array
};

// Dijkstra with priority queue (min-heap). Preserved original algorithm,
// upgraded to weighted edges + vehicle-aware walkability + route strategies.
class Dijkstra {
public:
    // Original unit-cost behaviour: every step costs 1.
    static DijkstraResult findPath(const DisasterMap& map, Position start, Position goal);
    // Weighted + strategy + vehicle kind ("Ambulance","RescueVan","FireTruck","Helicopter").
    static DijkstraResult findPath(const DisasterMap& map, Position start, Position goal,
                                   RouteMode mode, const std::string& vehicleKind);
    static DijkstraResult findPath(const DisasterMap& map, Position start, Position goal,
                                   const RouteStrategy& strategy, const std::string& vehicleKind);
    static int manhattan(Position a, Position b) { return std::abs(a.row - b.row) + std::abs(a.col - b.col); }
};

} // namespace drc

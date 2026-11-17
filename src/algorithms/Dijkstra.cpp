#include "../../include/algorithms/Dijkstra.hpp"
#include "../../include/vehicles/Vehicle.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <utility>

namespace drc {

namespace {
struct Node {
    int distance;
    Position position;
    bool operator<(const Node& other) const { return distance > other.distance; } // min-heap
};
constexpr int INF = std::numeric_limits<int>::max() / 4;

template <typename CanEnter, typename MoveCost>
DijkstraResult runDijkstra(const DisasterMap& map, Position start, Position goal,
                           const RouteStrategy& strategy, CanEnter canEnter, MoveCost moveCost) {
    DijkstraResult result;
    const int rows = map.rows();
    const int cols = map.cols();
    result.dist.assign(rows, std::vector<int>(cols, INF));
    result.pred.assign(rows, std::vector<Position>(cols, Position{-1, -1}));
    if (!map.inBounds(start) || !map.inBounds(goal) || !canEnter(goal)) return result;

    std::priority_queue<Node> pending;
    result.dist[start.row][start.col] = 0;
    pending.push({0, start});
    std::vector<std::vector<bool>> finalized(rows, std::vector<bool>(cols, false));

    while (!pending.empty()) {
        const Node current = pending.top();
        pending.pop();
        const Position from = current.position;
        if (finalized[from.row][from.col] || current.distance != result.dist[from.row][from.col]) continue;
        finalized[from.row][from.col] = true;
        result.visitedOrder.push_back(from);
        if (from == goal) break;

        for (const Position to : map.neighbors(from)) {
            if (!canEnter(to)) continue;
            int edge = strategy.edgeCost(from, to, map);
            int vehicleCost = moveCost(to);
            // Both components represent a minimum traversal cost for this edge.
            // The strategy may add avoidance/risk penalties; vehicle polymorphism
            // enforces special terrain costs such as a FireTruck entering fire.
            edge = std::max(0, edge);
            vehicleCost = std::max(0, vehicleCost);
            const int weight = strategy.optimizesFuel() ? edge : std::max(edge, vehicleCost);
            if (result.dist[from.row][from.col] + weight < result.dist[to.row][to.col]) {
                result.dist[to.row][to.col] = result.dist[from.row][from.col] + weight;
                result.pred[to.row][to.col] = from;
                pending.push({result.dist[to.row][to.col], to});
            }
        }
    }

    if (result.dist[goal.row][goal.col] == INF) return result;
    result.found = true;
    result.distance = result.dist[goal.row][goal.col];
    Position current = goal;
    std::vector<Position> reversed;
    while (!(current == start)) {
        reversed.push_back(current);
        current = result.pred[current.row][current.col];
        if (current.row == -1) { result.found = false; return result; }
    }
    reversed.push_back(start);
    std::reverse(reversed.begin(), reversed.end());
    result.path = std::move(reversed);
    result.steps = static_cast<int>(result.path.size()) - 1;
    return result;
}
}

DijkstraResult Dijkstra::findPath(const DisasterMap& map, Position start, Position goal) {
    return findPath(map, start, goal, RouteMode::Fastest, "Ambulance");
}

DijkstraResult Dijkstra::findPath(const DisasterMap& map, Position start, Position goal,
                                  RouteMode mode, const std::string& vehicleKind) {
    if (mode == RouteMode::Safest) return findPath(map, start, goal, SafestRouteStrategy{}, vehicleKind);
    if (mode == RouteMode::LowFuel) return findPath(map, start, goal, LowFuelRouteStrategy{}, vehicleKind);
    return findPath(map, start, goal, FastestRouteStrategy{}, vehicleKind);
}

DijkstraResult Dijkstra::findPath(const DisasterMap& map, Position start, Position goal,
                                  const RouteStrategy& strategy, const std::string& vehicleKind) {
    const auto canEnter = [&map, &vehicleKind](Position p) { return map.isWalkableFor(p, vehicleKind); };
    const auto moveCost = [&map, &vehicleKind](Position p) {
        return vehicleKind == "FireTruck" && map.getCell(p) == CellType::Fire ? 2 : 1;
    };
    return runDijkstra(map, start, goal, strategy, canEnter, moveCost);
}

DijkstraResult Dijkstra::findPath(const DisasterMap& map, Position start, Position goal,
                                  RouteMode mode, const Vehicle& vehicle) {
    if (mode == RouteMode::Safest) return findPath(map, start, goal, SafestRouteStrategy{}, vehicle);
    if (mode == RouteMode::LowFuel) return findPath(map, start, goal, LowFuelRouteStrategy{}, vehicle);
    return findPath(map, start, goal, FastestRouteStrategy{}, vehicle);
}

DijkstraResult Dijkstra::findPath(const DisasterMap& map, Position start, Position goal,
                                  const RouteStrategy& strategy, const Vehicle& vehicle) {
    const auto canEnter = [&map, &vehicle](Position p) { return vehicle.canEnter(map.getCell(p)); };
    const auto moveCost = [&map, &vehicle](Position p) { return vehicle.moveCost(map.getCell(p)); };
    return runDijkstra(map, start, goal, strategy, canEnter, moveCost);
}

} // namespace drc

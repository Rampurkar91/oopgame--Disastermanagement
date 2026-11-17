#include "../../include/algorithms/Dijkstra.hpp"
#include <queue>
#include <limits>
#include <algorithm>

namespace drc {

namespace {
struct Node {
    int dist;
    Position pos;
    bool operator<(Node const& o) const { return dist > o.dist; } // for min-heap
};
const int INF = std::numeric_limits<int>::max() / 4;
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
    DijkstraResult res;
    int R = map.rows(), C = map.cols();
    res.dist.assign(R, std::vector<int>(C, INF));
    res.pred.assign(R, std::vector<Position>(C, Position{-1,-1}));
    if (!map.inBounds(start) || !map.inBounds(goal)) return res;
    if (!map.isWalkableFor(start, vehicleKind) && start != goal) {
        // Start may be on a facility/house edge; allow starting position regardless.
    }
    if (!map.isWalkableFor(goal, vehicleKind)) return res; // invalid destination rejected

    std::priority_queue<Node> pq;
    res.dist[start.row][start.col] = 0;
    pq.push({0, start});
    std::vector<std::vector<bool>> done(R, std::vector<bool>(C, false));

    while (!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        Position u = cur.pos;
        if (done[u.row][u.col]) continue;
        if (cur.dist != res.dist[u.row][u.col]) continue;
        done[u.row][u.col] = true;
        res.visitedOrder.push_back(u);
        if (u == goal) break;
        for (Position v : map.neighbors(u)) {
            if (!map.isWalkableFor(v, vehicleKind)) continue;
            int w = strategy.edgeCost(u, v, map);
            if (w < 0) w = 0; // Dijkstra requires non-negative weights
            if (res.dist[u.row][u.col] + w < res.dist[v.row][v.col]) {
                res.dist[v.row][v.col] = res.dist[u.row][u.col] + w;
                res.pred[v.row][v.col] = u;
                pq.push({res.dist[v.row][v.col], v});
            }
        }
    }
    if (res.dist[goal.row][goal.col] == INF) return res;
    res.found = true;
    res.distance = res.dist[goal.row][goal.col];
    // Reconstruct path via predecessor array.
    Position cur = goal;
    std::vector<Position> rev;
    while (!(cur == start)) {
        rev.push_back(cur);
        cur = res.pred[cur.row][cur.col];
        if (cur.row == -1) { res.found = false; return res; }
    }
    rev.push_back(start);
    std::reverse(rev.begin(), rev.end());
    res.path = rev;
    res.steps = (int)rev.size() - 1;
    return res;
}

} // namespace drc

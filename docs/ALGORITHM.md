# Dijkstra Algorithm

Original design preserved: priority queue (min-heap), distance array, predecessor array, 4-directional grid movement.

## Pseudocode
```
dist[*] = INF; dist[start] = 0; PQ.push(start)
while PQ not empty:
    u = PQ.popMin()
    if u == goal: break
    for v in neighbors(u) where walkable(v):
        if dist[u] + w(u,v) < dist[v]:
            dist[v] = dist[u] + w(u,v); pred[v] = u; PQ.push(v)
path = reconstruct via pred from goal to start
```

## Complexity
Time `O((V+E) log V)`, space `O(V)`. V = 117 cells.

## Route modes (weighted upgrade)
- **FASTEST**: road=1, house=2, fire=3.
- **SAFEST**: base 1 + hazard penalties + proximity penalties near fire/flood.
- **LOW_FUEL**: road/hospital/station=1, house=3, fire=5.

All weights are non-negative, so Dijkstra remains correct. The visualizer shows the real `visitedOrder`, `dist`, and `pred` from the C++ run.

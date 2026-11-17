#pragma once
#include "../core/GameState.hpp"
#include "../map/DisasterMap.hpp"
#include "../core/Position.hpp"

namespace drc {

// Strategy pattern: each route mode defines edge cost.
class RouteStrategy {
public:
    virtual ~RouteStrategy() = default;
    virtual int edgeCost(Position from, Position to, const DisasterMap& map) const = 0;
    virtual RouteMode mode() const = 0;
    virtual bool optimizesFuel() const { return false; }
};
class FastestRouteStrategy : public RouteStrategy {
public:
    int edgeCost(Position, Position to, const DisasterMap& map) const override;
    RouteMode mode() const override { return RouteMode::Fastest; }
};
class SafestRouteStrategy : public RouteStrategy {
public:
    int edgeCost(Position from, Position to, const DisasterMap& map) const override;
    RouteMode mode() const override { return RouteMode::Safest; }
};
class LowFuelRouteStrategy : public RouteStrategy {
public:
    int edgeCost(Position, Position to, const DisasterMap& map) const override;
    RouteMode mode() const override { return RouteMode::LowFuel; }
    bool optimizesFuel() const override { return true; }
};

} // namespace drc

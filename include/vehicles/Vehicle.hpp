#pragma once
#include "../core/Position.hpp"
#include "../core/GameState.hpp"
#include "../map/DisasterMap.hpp"
#include <string>
#include <memory>

namespace drc {

// Abstract base (preserved original class name).
class Vehicle {
public:
    Vehicle(int id, std::string name, std::string kind, Position pos, int fuel, int maxFuel, int supplies)
        : id_(id), name_(std::move(name)), kind_(std::move(kind)), position_(pos),
          fuel_(fuel), maxFuel_(maxFuel), supplies_(supplies) {}
    virtual ~Vehicle() = default;
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& kind() const { return kind_; }
    Position position() const { return position_; }
    void setPosition(Position p) { position_ = p; }
    int fuel() const { return fuel_; }
    int maxFuel() const { return maxFuel_; }
    int supplies() const { return supplies_; }
    VehicleStatus status() const { return status_; }
    void setStatus(VehicleStatus s) { status_ = s; }
    bool isAerial() const { return kind_ == "Helicopter"; }
    // Polymorphic movement rules.
    virtual bool canEnter(CellType cell) const = 0;
    virtual int moveCost(CellType cell) const { (void)cell; return 1; }
    virtual std::string description() const = 0;
    bool hasFuelFor(int steps) const { return fuel_ >= steps; }
    bool consumeFuel(int steps) {
        if (fuel_ < steps) return false;
        fuel_ -= steps;
        if (fuel_ <= 0) { fuel_ = 0; status_ = VehicleStatus::OutOfFuel; }
        return true;
    }
    bool useSupply() {
        if (supplies_ <= 0) return false;
        --supplies_;
        return true;
    }
    void refuel() { fuel_ = maxFuel_; if (status_ == VehicleStatus::OutOfFuel) status_ = VehicleStatus::Idle; }
    double fuelPercent() const { return maxFuel_ ? (100.0 * fuel_ / maxFuel_) : 0.0; }
protected:
    int id_;
    std::string name_;
    std::string kind_;
    Position position_;
    int fuel_;
    int maxFuel_;
    int supplies_;
    VehicleStatus status_ = VehicleStatus::Idle;
};

class Ambulance : public Vehicle {
public:
    Ambulance(int id, Position p) : Vehicle(id, "Ambulance", "Ambulance", p, 70, 70, 6) {}
    bool canEnter(CellType c) const override {
        return c != CellType::Blocked && c != CellType::Flood && c != CellType::Fire;
    }
    std::string description() const override { return "Fast medical transport. Cannot enter fire/flood/blocked cells."; }
};
class RescueVan : public Vehicle {
public:
    RescueVan(int id, Position p) : Vehicle(id, "Rescue Van", "RescueVan", p, 90, 90, 3) {}
    bool canEnter(CellType c) const override {
        return c != CellType::Blocked && c != CellType::Flood && c != CellType::Fire;
    }
    std::string description() const override { return "High fuel, low supplies. Cannot enter fire/flood/blocked cells."; }
};
class FireTruck : public Vehicle {
public:
    FireTruck(int id, Position p) : Vehicle(id, "Fire Truck", "FireTruck", p, 100, 100, 2) {}
    // Specialized fire-response: may enter FIRE cells, still blocked by flood/blocked.
    bool canEnter(CellType c) const override {
        return c != CellType::Blocked && c != CellType::Flood;
    }
    int moveCost(CellType c) const override { return c == CellType::Fire ? 2 : 1; }
    std::string description() const override { return "Fire-response vehicle. May enter FIRE cells (cost 2)."; }
};
class Helicopter : public Vehicle {
public:
    Helicopter(int id, Position p) : Vehicle(id, "Helicopter", "Helicopter", p, 100, 100, 8) {}
    // Aerial: flies over all terrain.
    bool canEnter(CellType) const override { return true; }
    std::string description() const override { return "Aerial unit. Flies over all terrain types."; }
};

// Factory (Factory pattern).
class VehicleFactory {
public:
    static std::unique_ptr<Vehicle> create(int id, const std::string& kind, Position p);
    static std::vector<std::unique_ptr<Vehicle>> createDefaultFleet();
};

} // namespace drc

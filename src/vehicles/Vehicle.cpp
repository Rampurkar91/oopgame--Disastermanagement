#include "../../include/vehicles/Vehicle.hpp"
namespace drc {
std::unique_ptr<Vehicle> VehicleFactory::create(int id, const std::string& kind, Position p) {
    if (kind == "Ambulance") return std::make_unique<Ambulance>(id, p);
    if (kind == "RescueVan") return std::make_unique<RescueVan>(id, p);
    if (kind == "FireTruck") return std::make_unique<FireTruck>(id, p);
    if (kind == "Helicopter") return std::make_unique<Helicopter>(id, p);
    return std::make_unique<Ambulance>(id, p);
}
std::vector<std::unique_ptr<Vehicle>> VehicleFactory::createDefaultFleet() {
    std::vector<std::unique_ptr<Vehicle>> v;
    v.push_back(std::make_unique<Ambulance>(1, Position{1,1}));
    v.push_back(std::make_unique<RescueVan>(2, Position{1,2}));
    v.push_back(std::make_unique<FireTruck>(3, Position{2,7}));
    v.push_back(std::make_unique<Helicopter>(4, Position{4,6}));
    return v;
}
} // namespace drc

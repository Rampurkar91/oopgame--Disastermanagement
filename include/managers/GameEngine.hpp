#pragma once
#include "../core/Position.hpp"
#include "../core/GameState.hpp"
#include "../map/DisasterMap.hpp"
#include "../entities/Person.hpp"
#include "../vehicles/Vehicle.hpp"
#include "../facilities/Facility.hpp"
#include "../algorithms/Dijkstra.hpp"
#include <vector>
#include <memory>
#include <string>

namespace drc {

struct MissionConfig {
    int level = 1;
    std::string name = "Classic Mission";
    int timeLimit = 80;
    int fuelBonus = 0;
    int supplyBonus = 0;
    int floodCount = 6;
    int blockedCount = 8;
    int fireCount = 4;
};

struct RescueResult {
    bool success = false;
    std::string message;
    int scoreGained = 0;
};

// Central coordinator (Composition root). Owns all game state.
class GameEngine {
public:
    GameEngine();
    void newMission(int level);
    void reset();
    // Queries
    DisasterMap& map() { return map_; }
    const DisasterMap& map() const { return map_; }
    std::vector<std::unique_ptr<Vehicle>>& vehicles() { return vehicles_; }
    std::vector<Civilian>& civilians() { return civilians_; }
    std::vector<std::unique_ptr<Hospital>>& hospitals() { return hospitals_; }
    std::unique_ptr<FireStation>& fireStation() { return fireStation_; }
    Vehicle* selectedVehicle();
    const Vehicle* selectedVehicle() const;
    void selectVehicle(int id);
    int selectedVehicleId() const { return selectedVehicleId_; }
    int score() const { return score_; }
    int timeLeft() const { return timeLeft_; }
    bool paused() const { return paused_; }
    void setPaused(bool p) { paused_ = p; }
    int rescuedCount() const;
    int remainingCount() const;
    int level() const { return level_; }
    MissionConfig config() const { return config_; }
    DijkstraResult lastRoute() const { return lastRoute_; }
    RouteMode routeMode() const { return routeMode_; }
    void setRouteMode(RouteMode m) { routeMode_ = m; }
    // Actions
    DijkstraResult planRoute(Position goal);
    bool moveVehicleAlongRoute(); // moves one step; returns false when done/no route
    RescueResult rescueCivilian(int civilianId);
    RescueResult deliverToHospital(int civilianId, int hospitalId);
    RescueResult treatCivilian(int civilianId);
    bool refuelSelectedVehicle();
    void tick(); // one time unit; no-op when paused/over
    bool isVictory() const;
    bool isDefeat() const;
    std::vector<Civilian*> recommendedTargets();
    std::string statusSummary() const;
    // Save / load
    bool saveGame(const std::string& path) const;
    bool loadGame(const std::string& path);
private:
    DisasterMap map_;
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
    std::vector<Civilian> civilians_;
    std::vector<std::unique_ptr<Hospital>> hospitals_;
    std::unique_ptr<FireStation> fireStation_;
    int selectedVehicleId_ = 1;
    int score_ = 0;
    int timeLeft_ = 80;
    bool paused_ = false;
    int level_ = 1;
    MissionConfig config_;
    DijkstraResult lastRoute_;
    RouteMode routeMode_ = RouteMode::Fastest;
    Position lastGoal_{0,0};
    bool hasRoute_ = false;
    void spawnDefaultEntities();
    Vehicle* findVehicle(int id);
    Civilian* findCivilian(int id);
    Hospital* findHospital(int id);
};

} // namespace drc

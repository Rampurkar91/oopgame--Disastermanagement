// Disaster Rescue Commander - automated unit tests (no external framework).
#include "../include/map/DisasterMap.hpp"
#include "../include/algorithms/Dijkstra.hpp"
#include "../include/managers/GameEngine.hpp"
#include "../include/vehicles/Vehicle.hpp"
#include <iostream>
#include <cassert>

static int passed = 0, failed = 0;
#define CHECK(cond, name) do { if (cond) { ++passed; std::cout << "PASS: " << name << "\n"; } else { ++failed; std::cout << "FAIL: " << name << "\n"; } } while(0)

int main() {
    using namespace drc;
    // 1. Dijkstra finds a valid shortest path.
    {
        DisasterMap m; m.loadClassicMap();
        auto r = Dijkstra::findPath(m, {1,1}, {4,4});
        CHECK(r.found && r.path.front() == Position(1,1) && r.path.back() == Position(4,4) && r.steps == (int)r.path.size()-1, "1. Dijkstra valid path");
    }
    // 2. Blocked cells cannot be crossed.
    {
        DisasterMap m; m.loadClassicMap();
        m.setCell({2,2}, CellType::Blocked); m.setCell({2,3}, CellType::Blocked); m.setCell({3,2}, CellType::Blocked);
        auto r = Dijkstra::findPath(m, {1,1}, {2,2});
        CHECK(!r.found, "2. Blocked goal rejected");
    }
    // 3. Flooded cells cannot be crossed.
    {
        DisasterMap m; m.loadClassicMap();
        auto r = Dijkstra::findPath(m, {1,1}, {6,2}); // flood cell
        CHECK(!r.found, "3. Flood goal rejected for ground vehicle");
    }
    // 4. Fire rules: ground cannot enter, FireTruck can, Helicopter can.
    {
        DisasterMap m; m.loadClassicMap();
        auto ground = Dijkstra::findPath(m, {1,1}, {2,5}, RouteMode::Fastest, "Ambulance");
        auto fire = Dijkstra::findPath(m, {1,1}, {2,5}, RouteMode::Fastest, "FireTruck");
        auto heli = Dijkstra::findPath(m, {1,1}, {2,5}, RouteMode::Fastest, "Helicopter");
        CHECK(!ground.found && fire.found && heli.found, "4. Fire vehicle rules");
        CHECK(!m.isWalkable({2,5}), "4b. isWalkable excludes FIRE (bug fix)");
    }
    // 5. Invalid destinations rejected.
    {
        DisasterMap m; m.loadClassicMap();
        auto r = Dijkstra::findPath(m, {1,1}, {-1,99});
        CHECK(!r.found, "5. Out-of-bounds goal rejected");
    }
    // 6. Insufficient fuel prevents movement.
    {
        GameEngine e; e.newMission(1);
        auto* v = e.selectedVehicle();
        while (v->fuel() > 0) v->consumeFuel(1);
        e.planRoute({4,4});
        CHECK(!e.moveVehicleAlongRoute(), "6. No fuel -> no movement");
    }
    // 7. Fuel decreases correctly.
    {
        GameEngine e; e.newMission(1);
        auto* v = e.selectedVehicle();
        int before = v->fuel();
        e.planRoute({2,2});
        e.moveVehicleAlongRoute();
        CHECK(v->fuel() == before - 1, "7. Fuel -1 per step");
    }
    // 8. Medical supplies decrease correctly.
    {
        GameEngine e; e.newMission(1);
        auto* v = e.selectedVehicle();
        int before = v->supplies();
        e.planRoute({2,2});
        while (e.moveVehicleAlongRoute()) {}
        auto r = e.treatCivilian(1);
        CHECK(r.success && v->supplies() == before - 1, "8. Supplies -1 per treatment");
    }
    // 9. Civilians cannot be rescued twice.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2}); while (e.moveVehicleAlongRoute()) {}
        auto r1 = e.rescueCivilian(1);
        auto r2 = e.rescueCivilian(1);
        CHECK(r1.success && !r2.success, "9. Double rescue rejected");
    }
    // 10. Hospital capacity decreases correctly.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2}); while (e.moveVehicleAlongRoute()) {}
        e.rescueCivilian(1);
        e.planRoute({0,11}); while (e.moveVehicleAlongRoute()) {}
        auto r = e.deliverToHospital(1, 1);
        CHECK(r.success && e.hospitals()[0]->occupied() == 1, "10. Hospital admission occupies bed");
    }
    // 11. Full hospitals reject new admissions.
    {
        Hospital h(99, "Test", {0,0}, 1);
        CHECK(h.admit() && !h.admit(), "11. Full hospital rejects");
    }
    // 12. Fire station restores fuel.
    {
        GameEngine e; e.newMission(1);
        auto* v = e.selectedVehicle();
        v->consumeFuel(10);
        e.planRoute({2,8}); while (e.moveVehicleAlongRoute()) {}
        CHECK(e.refuelSelectedVehicle() && v->fuel() == v->maxFuel(), "12. Refuel restores fuel");
    }
    // 13. Score calculated correctly.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2}); while (e.moveVehicleAlongRoute()) {}
        auto r = e.rescueCivilian(1); // P1 priority 10
        CHECK(r.scoreGained == 10 && e.score() == 10, "13. Rescue score = priority");
    }
    // 14. Timer decreases during gameplay.
    {
        GameEngine e; e.newMission(1);
        int t0 = e.timeLeft(); e.tick();
        CHECK(e.timeLeft() == t0 - 1, "14. Timer ticks");
    }
    // 15. Timer stops during pause.
    {
        GameEngine e; e.newMission(1);
        e.setPaused(true); int t0 = e.timeLeft(); e.tick();
        CHECK(e.timeLeft() == t0, "15. Timer paused");
    }
    // 16. Mission completion works.
    {
        GameEngine e; e.newMission(1);
        for (auto& c : e.civilians()) c.setStatus(CivilianStatus::Admitted);
        CHECK(e.isVictory(), "16. Victory when all rescued");
    }
    // 17. Mission failure works.
    {
        GameEngine e; e.newMission(1);
        for (int i = 0; i < 100; ++i) e.tick();
        CHECK(e.isDefeat(), "17. Defeat on timeout");
    }
    // 18. Vehicle-specific movement works.
    {
        DisasterMap m; m.loadClassicMap();
        auto heli = Dijkstra::findPath(m, {1,1}, {6,2}, RouteMode::Fastest, "Helicopter"); // flood cell
        CHECK(heli.found, "18. Helicopter flies over flood");
    }
    // 19. Save and load work.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2}); while (e.moveVehicleAlongRoute()) {}
        e.rescueCivilian(1);
        CHECK(e.saveGame("saves/test_save.txt"), "19a. Save succeeds");
        GameEngine e2;
        CHECK(e2.loadGame("saves/test_save.txt") && e2.score() == 10, "19b. Load restores score");
    }
    // 20. Route modes produce valid paths.
    {
        DisasterMap m; m.loadClassicMap();
        auto a = Dijkstra::findPath(m, {1,1}, {7,10}, RouteMode::Fastest, "Ambulance");
        auto b = Dijkstra::findPath(m, {1,1}, {7,10}, RouteMode::Safest, "Ambulance");
        auto c = Dijkstra::findPath(m, {1,1}, {7,10}, RouteMode::LowFuel, "Ambulance");
        CHECK(a.found && b.found && c.found, "20. All route modes valid");
    }
    // Extra: original spec values preserved.
    {
        GameEngine e; e.newMission(1);
        CHECK(e.civilians().size() == 5 && e.civilians()[0].health() == 25 && e.civilians()[0].priority() == 10, "Extra: P1 spec");
        CHECK(e.timeLeft() == 80, "Extra: time 80");
        CHECK(e.hospitals()[0]->capacity() == 4 && e.hospitals()[1]->capacity() == 3, "Extra: hospital capacities");
        CHECK(e.vehicles().size() == 4, "Extra: 4 vehicles");
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}

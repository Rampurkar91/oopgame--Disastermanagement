// Disaster Rescue Commander - automated unit tests (no external framework).
#include "../include/map/DisasterMap.hpp"
#include "../include/algorithms/Dijkstra.hpp"
#include "../include/managers/GameEngine.hpp"
#include "../include/managers/SaveManager.hpp"
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
        FireTruck fireTruck(99, {1,1});
        Helicopter aircraft(100, {1,1});
        auto polymorphicFire = Dijkstra::findPath(m, {1,1}, {2,5}, RouteMode::Fastest, fireTruck);
        auto polymorphicAir = Dijkstra::findPath(m, {1,1}, {6,2}, RouteMode::Fastest, aircraft);
        CHECK(polymorphicFire.found && polymorphicFire.distance == 6 && polymorphicAir.found,
              "4c. Dijkstra dispatches vehicle movement rules polymorphically");
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
        CHECK(e2.loadGame("saves/test_save.txt") && e2.score() == 10 &&
              e2.civilians()[0].assignedVehicleId() == 1 &&
              e2.civilians()[0].status() == CivilianStatus::Rescued, "19b. Load restores score and patient assignment");
    }
    // 20. Route modes produce valid paths.
    {
        DisasterMap m; m.loadClassicMap();
        auto a = Dijkstra::findPath(m, {1,1}, {7,10}, RouteMode::Fastest, "Ambulance");
        auto b = Dijkstra::findPath(m, {1,1}, {7,10}, RouteMode::Safest, "Ambulance");
        auto c = Dijkstra::findPath(m, {1,1}, {7,10}, RouteMode::LowFuel, "Ambulance");
        CHECK(a.found && b.found && c.found && c.distance == c.steps,
              "20. All route modes valid and LOW_FUEL counts cells");
    }
    // 21. Treating a civilian is not the same as rescuing them.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2}); while (e.moveVehicleAlongRoute()) {}
        auto treated = e.treatCivilian(1);
        CHECK(treated.success && e.civilians()[0].status() == CivilianStatus::Treated &&
              !e.civilians()[0].isRescued() && e.remainingCount() == 5 && !e.isVictory(),
              "21. Treatment does not complete rescue");
        auto rescued = e.rescueCivilian(1);
        CHECK(rescued.success && e.civilians()[0].isRescued(), "21b. Treated civilian can still be rescued");
    }
    // 22. A planned route cannot be reused after selecting a different vehicle.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2});
        e.selectVehicle(2);
        Position before = e.selectedVehicle()->position();
        CHECK(!e.moveVehicleAlongRoute() && e.selectedVehicle()->position() == before,
              "22. Vehicle selection invalidates old route");
    }
    // 23. Settings key=value configuration loads and round-trips.
    {
        SettingsManager settings;
        CHECK(settings.load("config/settings.txt") && settings.volume == 70 && settings.soundOn &&
              settings.musicOn && settings.graphicsQuality == 1 && settings.animationSpeed == 1.0,
              "23. Settings parse key=value file");
        settings.volume = 35;
        settings.animationSpeed = 1.5;
        CHECK(settings.save("saves/test_settings.txt"), "23b. Settings save succeeds");
        SettingsManager restored;
        CHECK(restored.load("saves/test_settings.txt") && restored.volume == 35 &&
              restored.animationSpeed == 1.5, "23c. Settings round-trip");
    }
    // 24. Corrupt save data fails safely without mutating the mission.
    {
        GameEngine e; e.newMission(2);
        const int beforeScore = e.score();
        SaveManager::saveText("saves/bad_save.txt", "level=bad\nscore=1\n");
        CHECK(!e.loadGame("saves/bad_save.txt") && e.level() == 2 && e.score() == beforeScore,
              "24. Invalid save rejected safely");
    }
    // 25. Every built-in level keeps civilian and facility routes reachable for ground units.
    {
        bool reachable = true;
        GameEngine e;
        for (int level = 1; level <= 5; ++level) {
            e.newMission(level);
            const Position start = e.vehicles()[0]->position();
            for (const auto& civilian : e.civilians())
                reachable = reachable && Dijkstra::findPath(e.map(), start, civilian.position(),
                    RouteMode::Fastest, "Ambulance").found;
            for (const auto& hospital : e.hospitals())
                reachable = reachable && Dijkstra::findPath(e.map(), start, hospital->position(),
                    RouteMode::Fastest, "Ambulance").found;
        }
        DisasterMap randomized;
        randomized.randomize(5, 12345);
        for (Position goal : {Position{2,2}, Position{4,4}, Position{6,6}, Position{3,10},
                              Position{7,7}, Position{0,11}, Position{8,11}, Position{2,8}})
            reachable = reachable && Dijkstra::findPath(randomized, {1,1}, goal,
                RouteMode::Fastest, "Ambulance").found;
        CHECK(reachable, "25. Mission objectives stay reachable on all levels and randomized maps");
    }
    // 26. A rescued patient can only be delivered by the assigned rescue unit.
    {
        GameEngine e; e.newMission(1);
        e.planRoute({2,2}); while (e.moveVehicleAlongRoute()) {}
        e.rescueCivilian(1);
        e.selectVehicle(2);
        e.selectedVehicle()->setPosition({0,11});
        const auto result = e.deliverToHospital(1, 1);
        CHECK(!result.success && e.hospitals()[0]->occupied() == 0,
              "26. Patient assignment is enforced at delivery");
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

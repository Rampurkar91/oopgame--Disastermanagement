# Disaster Rescue Commander

**Intelligent City Disaster Simulation, Emergency Rescue and Resource Management System.**

A C++17 / SFML desktop game built with strong OOP. Players command emergency vehicles through a disaster-affected 9×13 city, rescue civilians, manage fuel and medical supplies, deliver patients to hospitals, and beat the clock — all routed by **Dijkstra's shortest-path algorithm**.

> **Note on the original repository:** the cloned repo contained only a one-line `README.md` (no source files, no commits beyond the initial commit). There was therefore no original C++ code to preserve. This implementation builds the full project from scratch to the requested specification: 9×13 map, P1–P5 civilians, 4 vehicles, 2 hospitals + fire station, time 80, and the original Dijkstra design (priority queue + distance/predecessor arrays), upgraded with weighted edges and route strategies.

## Features

- Main menu: Start / Continue / Level Selection / How to Play / DSA Visualizer / Statistics / Settings / Exit — every button works.
- 5 difficulty levels (Beginner → Survival) + classic 9×13 mission.
- Interactive map: click a civilian, hospital or cell to plan; stepwise vehicle movement and a highlighted route.
- Vehicle hierarchy: `Vehicle` → `Ambulance`, `RescueVan`, `FireTruck`, `Helicopter` (aerial movement).
- Civilian rescue system with priority-based recommendations, treatment (+20), hospital admission (+50).
- Hospitals with capacity limits; fire station refuelling.
- **Dijkstra visualizer** using real algorithm state (visited order, distances, predecessors, path) + FASTEST / SAFEST / LOW_FUEL strategies.
- Save/load, persistent sound and speed settings, rescue particles, pause/resume, timer, score.
- Optional web companion dashboard (`assets/web/dashboard.html`) reading the engine save file — no duplicated game state.
- 36 automated checks, including all 20 core cases plus polymorphic vehicle routing, settings round-trip, treatment/rescue separation, vehicle-route isolation, corrupt-save handling, level reachability and assigned-vehicle delivery.

## Technologies

C++17, STL, SFML (graphics/window/audio), CMake. No web server, no database, no internet required to play.

## Build & Run

### Linux (Debian/Ubuntu)
```bash
sudo apt install g++ cmake libsfml-dev
cmake -S . -B build
cmake --build build --config Release
./build/DisasterRescueCommander
```

### Windows (MinGW)
Install MinGW-w64, CMake, SFML. Then:
```bash
cmake -S . -B build
cmake --build build --config Release
build\DisasterRescueCommander.exe
```
Keep `assets/` and `config/` next to the executable (CMake copies them automatically) plus the SFML DLLs.

### Headless (no SFML)
If SFML is not found, CMake builds a headless simulation demo automatically:
```bash
cmake -S . -B build -DBUILD_GUI=OFF
cmake --build build
./build/DisasterRescueCommander
```

## Tests
```bash
cmake --build build
./build/DisasterTests
# or: ctest --test-dir build
```

## Controls
Mouse: click a civilian list row or map cell to route; click a hospital to set a delivery target. Use the bottom action bar to select a vehicle and rescue, treat, deliver, refuel, pause, save, or open the visualizer. Keys: `Space` pause, `1–4` select a vehicle, `Esc` return to menu.

## Companion dashboard
Open `assets/web/dashboard.html` in a browser and choose **Open saves/save.txt** to view the latest engine save. No server or second game engine is needed. The refresh button is optional when the project is hosted over HTTP.

## OOP Concepts
Encapsulation, inheritance (Vehicle/Facility/Person hierarchies), polymorphism (virtual `canEnter`, `edgeCost`), abstraction (RouteStrategy), composition (GameEngine), Factory (VehicleFactory), Strategy (route modes), smart pointers (`unique_ptr`), operator overloading (`Position`).

## Docs
- `docs/ARCHITECTURE.md` — class diagram & responsibilities
- `docs/ALGORITHM.md` — Dijkstra explanation, complexity, route modes
- `docs/DESIGN.md` — 12-colour design system

## Build verification and limitations
- The headless engine/demo and 36 checks build and pass locally. SFML 2.6 headers were used to syntax-check every GUI translation unit; a full local GUI link/run could not be verified because the sandbox lacks SFML development libraries. `.github/workflows/build.yml` installs SFML and builds the GUI on GitHub Actions.
- SFML is required for the graphical interface; without it, CMake builds the headless demo.
- Sound effect files are optional (`assets/sounds/*.wav`); missing clips are skipped silently. A DejaVu Sans font and its license are included under `assets/fonts/`.
- Save format is plain text (documented in `docs/ARCHITECTURE.md`).

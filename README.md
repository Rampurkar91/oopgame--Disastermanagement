# Disaster Rescue Commander

**Intelligent City Disaster Simulation, Emergency Rescue and Resource Management System.**

A C++17 / SFML desktop game built with strong OOP. Players command emergency vehicles through a disaster-affected 9×13 city, rescue civilians, manage fuel and medical supplies, deliver patients to hospitals, and beat the clock — all routed by **Dijkstra's shortest-path algorithm**.

> **Note on the original repository:** the cloned repo contained only a one-line `README.md` (no source files, no commits beyond the initial commit). There was therefore no original C++ code to preserve. This implementation builds the full project from scratch to the specification in the issue: 9×13 map, P1–P5 civilians, 4 vehicles, 2 hospitals + fire station, time 80, and the original Dijkstra design (priority queue + distance/predecessor arrays), upgraded with weighted edges and route strategies.

## Features

- Main menu: Start / Continue / Level Selection / How to Play / DSA Visualizer / Statistics / Settings / Exit — every button works.
- 5 difficulty levels (Beginner → Survival) + classic 9×13 mission.
- Interactive map: click to plan routes, animated route lines, zoom-ready renderer, legend.
- Vehicle hierarchy: `Vehicle` → `Ambulance`, `RescueVan`, `FireTruck`, `Helicopter` (aerial movement).
- Civilian rescue system with priority-based recommendations, treatment (+20), hospital admission (+50).
- Hospitals with capacity limits; fire station refuelling.
- **Dijkstra visualizer** using real algorithm state (visited order, distances, predecessors, path) + FASTEST / SAFEST / LOW_FUEL strategies.
- Save/load, settings, particles, sound hooks, pause/resume, timer, score.
- Optional web companion dashboard (`assets/web/dashboard.html`) reading the engine save file — no duplicated game state.
- Automated unit tests covering all 20 required cases.

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
Mouse: click map cell = plan route; bottom bar = actions. Keys: `Space` pause, `1–4` select vehicle, `Esc` menu.

## OOP Concepts
Encapsulation, inheritance (Vehicle/Facility/Person hierarchies), polymorphism (virtual `canEnter`, `edgeCost`), abstraction (RouteStrategy), composition (GameEngine), Factory (VehicleFactory), Strategy (route modes), smart pointers (`unique_ptr`), operator overloading (`Position`).

## Docs
- `docs/ARCHITECTURE.md` — class diagram & responsibilities
- `docs/ALGORITHM.md` — Dijkstra explanation, complexity, route modes
- `docs/DESIGN.md` — 12-colour design system

## Known limitations
- SFML GUI requires SFML installed; otherwise headless mode runs.
- Sound files are optional (`assets/sounds/*.wav`); missing files are skipped silently.
- Save format is a simple text format (documented in `docs/ARCHITECTURE.md`).

# Architecture

## Layers
- **Core** (`include/core`): `Position`, `GameState` enums.
- **Map** (`include/map`): `DisasterMap` — 9×13 grid, cell types, walkability.
- **Entities** (`include/entities`): `Person` → `Civilian`.
- **Vehicles** (`include/vehicles`): `Vehicle` → Ambulance/RescueVan/FireTruck/Helicopter + `VehicleFactory`.
- **Facilities** (`include/facilities`): `Facility` → Hospital/FireStation.
- **Algorithms** (`include/algorithms`): `Dijkstra` + `RouteStrategy` (Strategy pattern).
- **Managers** (`include/managers`): `GameEngine` (composition root), `SaveManager`, `SettingsManager`.
- **UI** (`include/ui`): SFML renderer, map renderer, particles, audio, theme.

## Key rules
- Ground vehicles (Ambulance/RescueVan) cannot enter BLOCKED/FLOOD/FIRE.
- FireTruck may enter FIRE (cost 2) — explicit fire-response behaviour.
- Helicopter flies over all terrain.
- One fuel per cell moved. Rescue score = priority. Treatment +20. Admission +50.
- Timer ticks only when not paused and mission is active.
- Treatment improves health and adds +20, but civilians still require rescue/admission to complete the mission.
- Rescued civilians are associated with their rescue vehicle; only that vehicle can deliver them.
- Generated hazards never occupy standard spawn/objective cells or disconnect those objectives from the ambulance start.

## Save format (`saves/save.txt`)
```
level=1
score=10
time=75
vehicle <id> <row> <col> <fuel> <supplies>
civilian <id> <health> <statusInt> <assignedVehicleId> <hospitalId>
hospital <id> <occupied>
```
Older saves without the optional assigned-vehicle and hospital fields remain readable.

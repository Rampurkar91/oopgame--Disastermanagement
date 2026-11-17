#include "../include/managers/GameEngine.hpp"
#include "../include/managers/SaveManager.hpp"
#include <iostream>
#ifdef DRC_HAS_SFML
#include "../include/ui/GameRenderer.hpp"
#endif

int main() {
    drc::SettingsManager settings;
    settings.load("config/settings.txt");
#ifdef DRC_HAS_SFML
    std::cout << "Disaster Rescue Commander - SFML GUI mode\n";
    drc::GameRenderer renderer;
    renderer.run();
#else
    // Headless fallback (no SFML in this environment): run a scripted simulation
    // so the engine is fully testable and demonstrable from the terminal.
    std::cout << "Disaster Rescue Commander - headless simulation mode (SFML not found at build time)\n";
    std::cout << "Build with SFML installed for the graphical interface.\n\n";
    drc::GameEngine engine;
    engine.newMission(1);
    std::cout << engine.statusSummary() << "\n";
    auto* v = engine.selectedVehicle();
    std::cout << "Selected: " << v->name() << " at " << v->position() << " fuel=" << v->fuel() << "\n";
    auto targets = engine.recommendedTargets();
    std::cout << "Recommended first target: " << targets[0]->name() << " urgency=" << targets[0]->urgencyScore() << "\n";
    auto route = engine.planRoute(targets[0]->position());
    std::cout << "Route found=" << route.found << " distance=" << route.distance << " steps=" << route.steps << "\n";
    int guard = 0;
    while (engine.moveVehicleAlongRoute() && guard++ < 200) {}
    std::cout << "Vehicle now at " << v->position() << " fuel=" << v->fuel() << "\n";
    auto rr = engine.rescueCivilian(targets[0]->id());
    std::cout << "Rescue: " << rr.message << "\n";
    engine.saveGame("saves/save.txt");
    std::cout << "Saved to saves/save.txt\n";
    std::cout << engine.statusSummary() << "\n";
    std::cout << "Headless demo complete. Run the GUI build for full gameplay.\n";
#endif
    return 0;
}

#ifdef DRC_HAS_SFML
#include "../../include/ui/GameRenderer.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace drc {

namespace {
const sf::Color Background = theme::MidnightNavyColor;
const sf::Color Cyan = theme::NeonCyanColor;
const sf::Color White(235, 242, 250); // neutral text for contrast on the dark background
const sf::Color Muted(150, 180, 200); // neutral secondary text
}

GameRenderer::GameRenderer() : window_(sf::VideoMode(1280, 800), "Disaster Rescue Commander") {
    window_.setFramerateLimit(60);
    settings_.load("config/settings.txt");
    audio_.setEnabled(settings_.soundOn);
    audio_.setVolume(settings_.volume);
    loadFont();
    mapRenderer_ = std::make_unique<MapRenderer>(sf::Vector2f(300.f, 90.f), 44.f);
}

bool GameRenderer::loadFont() {
    const char* candidates[] = {
        "assets/fonts/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "DejaVuSans.ttf"
    };
    for (const char* candidate : candidates)
        if (font_.loadFromFile(candidate)) return true;
    return false;
}

bool GameRenderer::inRect(sf::Vector2f p, float x, float y, float w, float h) const {
    return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
}

void GameRenderer::drawText(const std::string& value, float x, float y, unsigned size, sf::Color color) {
    if (!font_.getInfo().family.empty()) {
        sf::Text text;
        text.setFont(font_);
        text.setString(value);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
        window_.draw(text);
    }
}

void GameRenderer::drawButton(sf::Vector2f pos, sf::Vector2f size,
                              const std::string& label, bool selected) {
    sf::RectangleShape button(size);
    button.setPosition(pos);
    button.setFillColor(selected ? theme::TealColor : theme::ElectricBlueColor);
    button.setOutlineColor(Cyan);
    button.setOutlineThickness(1.5f);
    window_.draw(button);
    drawText(label, pos.x + 10.f, pos.y + 8.f, 15, White);
}

void GameRenderer::setFeedback(const std::string& message, bool success) {
    feedback_ = message;
    feedbackSeconds_ = 4.5f;
    feedbackSuccess_ = success;
}

bool GameRenderer::saveSettings() {
    if (settings_.save("config/settings.txt")) return true;
    setFeedback("Could not save settings to config/settings.txt.");
    return false;
}

void GameRenderer::routeTo(Position destination, const std::string& description) {
    selectedDestination_ = destination;
    hasSelectedDestination_ = true;
    const DijkstraResult route = engine_.planRoute(destination);
    if (!route.found) {
        setFeedback("No valid route to " + description + " for the selected vehicle.");
        return;
    }
    setFeedback("Route to " + description + ": " + std::to_string(route.steps) + " cells.", true);
}

void GameRenderer::selectMapCell(Position cell) {
    for (auto& civilian : engine_.civilians()) {
        if (civilian.position() == cell && !civilian.isRescued()) {
            selectedCivilianId_ = civilian.id();
            routeTo(cell, civilian.name());
            return;
        }
    }
    for (auto& hospital : engine_.hospitals()) {
        if (hospital->position() == cell) {
            selectedHospitalId_ = hospital->id();
            routeTo(cell, hospital->name());
            return;
        }
    }
    if (engine_.fireStation() && engine_.fireStation()->position() == cell) {
        routeTo(cell, engine_.fireStation()->name());
        return;
    }
    routeTo(cell, "cell " + cell.toString());
}

void GameRenderer::drawMainMenu() {
    window_.clear(Background);
    drawText("DISASTER RESCUE COMMANDER", 340.f, 55.f, 34, Cyan);
    drawText("Intelligent City Disaster Simulation", 420.f, 105.f, 18, White);
    const char* items[] = {
        "Start New Mission", "Continue Mission", "Level Selection", "How to Play",
        "Dijkstra Visualizer", "Mission Statistics", "Settings", "Exit Game"
    };
    for (int i = 0; i < 8; ++i)
        drawButton({440.f, 160.f + static_cast<float>(i) * 61.f}, {400.f, 47.f}, items[i]);
    drawText("C++17  |  SFML  |  Dijkstra shortest path  |  OOP architecture",
             370.f, 680.f, 14, Muted);
    if (feedbackSeconds_ > 0.f)
        drawText(feedback_, 300.f, 725.f, 13, feedbackSuccess_ ? sf::Color(139,255,90) : sf::Color(255,212,59));
}

void GameRenderer::drawLevelSelect() {
    window_.clear(Background);
    drawText("SELECT MISSION LEVEL", 420.f, 55.f, 30, Cyan);
    const char* names[] = {
        "Level 1 - Beginner (Classic)", "Level 2 - Intermediate",
        "Level 3 - Advanced", "Level 4 - Expert", "Level 5 - Survival"
    };
    for (int i = 0; i < 5; ++i)
        drawButton({380.f, 135.f + static_cast<float>(i) * 68.f}, {520.f, 52.f}, names[i]);
    drawText("Higher levels add hazards and reduce the mission timer.", 410.f, 500.f, 15, Muted);
    drawButton({380.f, 545.f}, {200.f, 46.f}, "Back");
}

void GameRenderer::drawDashboard() {
    window_.clear(Background);
    const Vehicle* vehicle = engine_.selectedVehicle();

    std::ostringstream hud;
    hud << engine_.statusSummary();
    if (vehicle) {
        hud << "  |  " << vehicle->name() << "  Fuel " << vehicle->fuel() << "/" << vehicle->maxFuel()
            << "  Supplies " << vehicle->supplies();
    }
    drawText(hud.str(), 16.f, 10.f, 15, White);
    drawText(engine_.paused() ? "PAUSED" : "MISSION ACTIVE", 16.f, 36.f, 13,
             engine_.paused() ? sf::Color(255,212,59) : sf::Color(22,199,132));

    // Civilian list: clicking a row selects that patient and plans a route.
    drawText("CIVILIANS (click to target)", 16.f, 70.f, 14, Cyan);
    float y = 94.f;
    for (const auto& civilian : engine_.civilians()) {
        std::ostringstream row;
        row << civilian.name() << "  H:" << civilian.health() << "  P:" << civilian.priority()
            << "  " << toString(civilian.status());
        sf::Color color = civilian.condition() == HealthCondition::Critical
                            ? sf::Color(240,68,82) : White;
        if (civilian.id() == selectedCivilianId_) color = Cyan;
        drawText(row.str(), 16.f, y, 11, color);
        y += 21.f;
    }
    drawText("PRIORITY TARGETS", 16.f, y + 8.f, 13, sf::Color(139,255,90));
    y += 29.f;
    const auto targets = engine_.recommendedTargets();
    for (const auto* civilian : targets) {
        drawText(civilian->name() + "  urgency " + std::to_string(civilian->urgencyScore()),
                 16.f, y, 11, Muted);
        y += 17.f;
    }

    // Interactive city map and current Dijkstra path.
    mapRenderer_->draw(window_, engine_, font_);
    const sf::Color legendColors[] = {theme::DeepBlueColor, theme::RoyalVioletColor,
        theme::ElectricBlueColor, theme::EmergencyRedColor, theme::TealColor, theme::AmberColor};
    const char* legendLabels[] = {"Road", "Blocked", "Flood", "Fire", "Hospital", "Station"};
    for (int i = 0; i < 6; ++i) {
        const float x = 300.f + static_cast<float>(i) * 92.f;
        sf::RectangleShape swatch({13.f, 13.f});
        swatch.setPosition(x, 505.f);
        swatch.setFillColor(legendColors[i]);
        window_.draw(swatch);
        drawText(legendLabels[i], x + 18.f, 503.f, 10, Muted);
    }

    drawText("SELECTED UNIT", 900.f, 70.f, 14, Cyan);
    if (vehicle) {
        std::ostringstream detail;
        detail << vehicle->name() << "\n" << vehicle->description()
               << "\nPosition " << vehicle->position().toString()
               << "\nFuel " << vehicle->fuel() << "/" << vehicle->maxFuel()
               << " (" << static_cast<int>(vehicle->fuelPercent()) << "%)"
               << "\nMedical supplies " << vehicle->supplies()
               << "\nStatus " << toString(vehicle->status());
        drawText(detail.str(), 900.f, 95.f, 12, White);
    }
    drawText("HOSPITALS", 900.f, 245.f, 14, sf::Color(0,191,166));
    float hospitalY = 270.f;
    for (const auto& hospital : engine_.hospitals()) {
        const bool chosen = hospital->id() == selectedHospitalId_;
        std::ostringstream line;
        line << hospital->name() << " " << hospital->occupied() << "/" << hospital->capacity()
             << " beds  " << hospital->position().toString();
        drawText(line.str(), 900.f, hospitalY, 11, chosen ? Cyan : White);
        hospitalY += 20.f;
    }
    drawText("Fire Station  " + engine_.fireStation()->position().toString(),
             900.f, hospitalY + 8.f, 11, sf::Color(255,159,28));
    drawText("Route mode: " + toString(engine_.routeMode()) + "  Cost: " +
             std::to_string(engine_.lastRoute().distance), 900.f, hospitalY + 32.f, 11, Muted);

    std::string selection = "Target: ";
    if (selectedCivilianId_ >= 0) selection += "P" + std::to_string(selectedCivilianId_);
    else selection += "none";
    if (hasSelectedDestination_) selection += "  |  Destination " + selectedDestination_.toString();
    drawText(selection, 300.f, 655.f, 12, Cyan);
    if (feedbackSeconds_ > 0.f)
        drawText(feedback_, 300.f, 678.f, 12,
                 feedbackSuccess_ ? sf::Color(139,255,90) : sf::Color(255,212,59));
    drawText("Click a patient, hospital or map cell to plan. Vehicle moves one cell per game-time step.",
             300.f, 701.f, 10, Muted);

    const char* actions[] = {"Vehicle", "Route", "Rescue", "Deliver", "Treat",
                             "Refuel", "Pause", "Visualizer", "Save", "Menu"};
    for (int i = 0; i < 10; ++i)
        drawButton({16.f + static_cast<float>(i) * 126.f, 744.f}, {118.f, 42.f}, actions[i]);
    particles_.draw(window_);
}

void GameRenderer::drawVisualizer() {
    window_.clear(Background);
    drawText("DIJKSTRA ROUTE VISUALIZER", 390.f, 18.f, 24, Cyan);
    mapRenderer_->draw(window_, engine_, font_);
    const DijkstraResult route = engine_.lastRoute();
    drawText("VISITED NODE  DIST  PREDECESSOR", 900.f, 82.f, 12, Cyan);
    float nodeY = 108.f;
    for (size_t i = 0; i < route.visitedOrder.size() && i < 14; ++i) {
        const Position node = route.visitedOrder[i];
        const int distance = route.dist.empty() ? -1 : route.dist[node.row][node.col];
        const Position predecessor = route.pred.empty() ? Position{-1, -1} : route.pred[node.row][node.col];
        std::ostringstream nodeInfo;
        nodeInfo << (i + 1) << ". " << node.toString() << "   " << distance << "   "
                 << (predecessor.row < 0 ? "START" : predecessor.toString());
        drawText(nodeInfo.str(), 900.f, nodeY, 11, White);
        nodeY += 22.f;
    }
    if (hasSelectedDestination_ && engine_.map().inBounds(selectedDestination_) && !route.dist.empty()) {
        const Position predecessor = route.pred[selectedDestination_.row][selectedDestination_.col];
        std::ostringstream goalInfo;
        goalInfo << "Goal " << selectedDestination_.toString() << "  dist "
                 << route.dist[selectedDestination_.row][selectedDestination_.col]
                 << "  pred " << (predecessor.row < 0 ? "none" : predecessor.toString());
        drawText(goalInfo.str(), 900.f, 430.f, 11, sf::Color(139,255,90));
    }
    std::ostringstream stats;
    stats << "Found: " << (route.found ? "yes" : "no")
          << "   Weighted cost: " << route.distance
          << "   Steps: " << route.steps
          << "   Finalized nodes: " << route.visitedOrder.size()
          << "   Mode: " << toString(engine_.routeMode());
    drawText(stats.str(), 300.f, 500.f, 13, White);
    drawText("Click any cell to run Dijkstra. Modes re-run the selected destination.",
             300.f, 525.f, 12, Muted);
    drawText("dist[*]=INF; dist[start]=0; while PQ: u=popMin(); relax each walkable neighbor v.",
             300.f, 550.f, 11, White);
    drawText("Complexity: O((V+E) log V) time, O(V) space; V = 117 grid cells.",
             300.f, 572.f, 11, White);
    drawText("Finalized order (first 18):", 300.f, 598.f, 11, sf::Color(139,255,90));
    std::string order;
    for (size_t i = 0; i < route.visitedOrder.size() && i < 18; ++i) {
        if (!order.empty()) order += "  ";
        order += route.visitedOrder[i].toString();
    }
    drawText(order, 300.f, 618.f, 11, White);
    const char* modes[] = {"FASTEST", "SAFEST", "LOW FUEL"};
    for (int i = 0; i < 3; ++i)
        drawButton({300.f + static_cast<float>(i) * 160.f, 675.f}, {150.f, 42.f}, modes[i],
                   static_cast<int>(engine_.routeMode()) == i);
    drawButton({820.f, 675.f}, {150.f, 42.f}, "Back");
    if (feedbackSeconds_ > 0.f)
        drawText(feedback_, 300.f, 650.f, 11, feedbackSuccess_ ? sf::Color(139,255,90) : sf::Color(255,212,59));
}

void GameRenderer::drawHelp() {
    window_.clear(Background);
    drawText("HOW TO PLAY", 500.f, 40.f, 30, Cyan);
    drawText("1. Select a vehicle with the Vehicle button or keys 1-4.\n"
             "2. Click a civilian in the list, on the map, or click any destination cell.\n"
             "3. The route is planned with Dijkstra; the selected vehicle moves along it over time.\n"
             "4. At a civilian, press Rescue. Treat first for +20 points and a health boost.\n"
             "5. Select a rescued civilian and click a hospital; press Deliver at the hospital (+50).\n"
             "6. Refuel only at the fire station (2,8). Movement consumes one fuel per cell.\n"
             "Ground vehicles avoid blocked, flooded and burning cells. Fire Trucks can enter fire;\n"
             "the Helicopter flies over all terrain. Space pauses; Escape returns to the menu.",
             205.f, 120.f, 15, White);
    drawButton({500.f, 520.f}, {280.f, 50.f}, "Back");
}

void GameRenderer::drawSettings() {
    window_.clear(Background);
    drawText("SETTINGS", 520.f, 55.f, 30, Cyan);
    drawText("Sound effects", 330.f, 160.f, 18, White);
    drawButton({770.f, 145.f}, {170.f, 48.f}, settings_.soundOn ? "ON" : "OFF", settings_.soundOn);
    drawText("Sound volume", 330.f, 245.f, 18, White);
    drawButton({700.f, 225.f}, {54.f, 46.f}, "-");
    drawText(std::to_string(settings_.volume) + "%", 780.f, 235.f, 18, Cyan);
    drawButton({850.f, 225.f}, {54.f, 46.f}, "+");
    std::ostringstream speed;
    speed << std::fixed << std::setprecision(2) << settings_.animationSpeed << "x";
    drawText("Vehicle / game speed", 330.f, 330.f, 18, White);
    drawButton({700.f, 310.f}, {54.f, 46.f}, "-");
    drawText(speed.str(), 780.f, 320.f, 18, Cyan);
    drawButton({850.f, 310.f}, {54.f, 46.f}, "+");
    drawText("Settings are saved to config/settings.txt.", 330.f, 390.f, 13, Muted);
    drawButton({500.f, 500.f}, {280.f, 50.f}, "Back");
}

void GameRenderer::drawStatistics() {
    window_.clear(Background);
    drawText("CURRENT MISSION STATISTICS", 360.f, 35.f, 28, Cyan);
    drawText(engine_.statusSummary(), 290.f, 95.f, 17, White);
    int admitted = 0, rescued = 0, treated = 0, waiting = 0;
    for (const auto& civilian : engine_.civilians()) {
        if (civilian.status() == CivilianStatus::Admitted) ++admitted;
        else if (civilian.status() == CivilianStatus::Rescued) ++rescued;
        else if (civilian.status() == CivilianStatus::Treated) ++treated;
        else ++waiting;
    }
    std::ostringstream civilianStats;
    civilianStats << "Civilians: " << engine_.civilians().size() << " total  |  " << rescued
                  << " rescued  |  " << admitted << " admitted  |  " << treated << " treated  |  "
                  << waiting << " waiting";
    drawText(civilianStats.str(), 290.f, 135.f, 14, White);
    drawText("VEHICLE RESOURCES", 290.f, 190.f, 16, Cyan);
    float y = 220.f;
    for (const auto& vehicle : engine_.vehicles()) {
        std::ostringstream row;
        row << vehicle->name() << "  fuel " << vehicle->fuel() << "/" << vehicle->maxFuel()
            << "  supplies " << vehicle->supplies() << "  status " << toString(vehicle->status());
        drawText(row.str(), 290.f, y, 13, White);
        y += 25.f;
    }
    drawText("HOSPITAL CAPACITY", 290.f, y + 12.f, 16, Cyan);
    y += 42.f;
    for (const auto& hospital : engine_.hospitals()) {
        std::ostringstream row;
        row << hospital->name() << "  " << hospital->occupied() << "/" << hospital->capacity()
            << " occupied  |  " << hospital->freeBeds() << " beds free";
        drawText(row.str(), 290.f, y, 13, White);
        y += 25.f;
    }
    drawText("Route strategy: " + toString(engine_.routeMode()), 290.f, y + 10.f, 13, Muted);
    drawButton({500.f, 650.f}, {280.f, 50.f}, "Back");
}

void GameRenderer::drawGameOver(bool victory) {
    window_.clear(Background);
    drawText(victory ? "MISSION COMPLETE!" : "MISSION FAILED", 400.f, 180.f, 36,
             victory ? sf::Color(139,255,90) : sf::Color(240,68,82));
    drawText(engine_.statusSummary(), 450.f, 250.f, 18, White);
    drawButton({450.f, 330.f}, {380.f, 50.f}, "Play Again");
    drawButton({450.f, 400.f}, {380.f, 50.f}, "Main Menu");
}

void GameRenderer::handleMenuClick(sf::Vector2f mouse) {
    for (int i = 0; i < 8; ++i) {
        if (!inRect(mouse, 440.f, 160.f + static_cast<float>(i) * 61.f, 400.f, 47.f)) continue;
        audio_.playClick();
        switch (i) {
            case 0:
                engine_.newMission(1);
                selectedCivilianId_ = -1;
                hasSelectedDestination_ = false;
                state_ = GameState::Playing;
                setFeedback("Level 1 mission started.", true);
                break;
            case 1:
                if (engine_.loadGame("saves/save.txt")) {
                    selectedCivilianId_ = -1;
                    hasSelectedDestination_ = false;
                    state_ = GameState::Playing;
                    setFeedback("Saved mission loaded.", true);
                } else {
                    setFeedback("No readable save found at saves/save.txt.");
                }
                break;
            case 2: state_ = GameState::LevelSelect; break;
            case 3: state_ = GameState::Help; break;
            case 4: {
                auto targets = engine_.recommendedTargets();
                if (!targets.empty()) {
                    selectedCivilianId_ = targets.front()->id();
                    routeTo(targets.front()->position(), targets.front()->name());
                }
                state_ = GameState::Visualizer;
                break;
            }
            case 5: state_ = GameState::Statistics; break;
            case 6: state_ = GameState::Settings; break;
            case 7: window_.close(); break;
        }
        return;
    }
}

void GameRenderer::handleDashboardClick(sf::Vector2f mouse) {
    // Civilian rows on the left are direct, keyboard-free target selection controls.
    if (inRect(mouse, 16.f, 94.f, 270.f, 5.f * 21.f)) {
        const int index = static_cast<int>((mouse.y - 94.f) / 21.f);
        if (index >= 0 && index < static_cast<int>(engine_.civilians().size())) {
            auto& civilian = engine_.civilians()[static_cast<size_t>(index)];
            selectedCivilianId_ = civilian.id();
            if (!civilian.isRescued()) routeTo(civilian.position(), civilian.name());
            else setFeedback(civilian.name() + " is rescued; choose a hospital to deliver.", true);
            return;
        }
    }

    // Ten action buttons at the bottom of the dashboard.
    for (int action = 0; action < 10; ++action) {
        const float x = 16.f + static_cast<float>(action) * 126.f;
        if (!inRect(mouse, x, 744.f, 118.f, 42.f)) continue;
        audio_.playClick();
        Vehicle* vehicle = engine_.selectedVehicle();
        switch (action) {
            case 0: {
                const int nextId = engine_.selectedVehicleId() % 4 + 1;
                engine_.selectVehicle(nextId);
                vehicle = engine_.selectedVehicle();
                setFeedback("Selected " + vehicle->name() + ". Choose a target and re-plan.", true);
                break;
            }
            case 1: {
                if (hasSelectedDestination_) {
                    routeTo(selectedDestination_, "selected destination");
                } else {
                    auto targets = engine_.recommendedTargets();
                    if (targets.empty()) setFeedback("No civilians remain to target.");
                    else {
                        selectedCivilianId_ = targets.front()->id();
                        routeTo(targets.front()->position(), targets.front()->name());
                    }
                }
                break;
            }
            case 2: {
                Civilian* target = nullptr;
                for (auto& civilian : engine_.civilians())
                    if (civilian.id() == selectedCivilianId_ && !civilian.isRescued()) target = &civilian;
                if (!target && vehicle)
                    for (auto& civilian : engine_.civilians())
                        if (civilian.position() == vehicle->position() && !civilian.isRescued()) { target = &civilian; break; }
                if (!target) {
                    auto targets = engine_.recommendedTargets();
                    if (!targets.empty()) target = targets.front();
                }
                if (!target) { setFeedback("There is no waiting civilian to rescue."); break; }
                selectedCivilianId_ = target->id();
                if (!vehicle || !(vehicle->position() == target->position())) {
                    routeTo(target->position(), target->name());
                    setFeedback("Route planned to " + target->name() + ". Press Rescue when the vehicle arrives.");
                    break;
                }
                const RescueResult result = engine_.rescueCivilian(target->id());
                setFeedback(result.message, result.success);
                if (result.success) {
                    audio_.playRescue();
                    particles_.emit({600.f, 350.f}, 20, sf::Color(139,255,90));
                }
                break;
            }
            case 3: {
                Civilian* patient = nullptr;
                for (auto& civilian : engine_.civilians())
                    if (civilian.id() == selectedCivilianId_ && civilian.status() == CivilianStatus::Rescued)
                        patient = &civilian;
                if (!patient) {
                    for (auto& civilian : engine_.civilians())
                        if (civilian.status() == CivilianStatus::Rescued &&
                            (civilian.assignedVehicleId() < 0 || !vehicle || civilian.assignedVehicleId() == vehicle->id())) {
                            patient = &civilian;
                            break;
                        }
                }
                if (!patient) { setFeedback("Rescue a civilian before delivering them."); break; }
                selectedCivilianId_ = patient->id();
                if (patient->assignedVehicleId() > 0 && vehicle && patient->assignedVehicleId() != vehicle->id()) {
                    engine_.selectVehicle(patient->assignedVehicleId());
                    vehicle = engine_.selectedVehicle();
                    setFeedback("Switched to the vehicle carrying " + patient->name() + ".", true);
                }
                Hospital* hospital = nullptr;
                for (auto& candidate : engine_.hospitals())
                    if (candidate->id() == selectedHospitalId_ && candidate->hasSpace()) hospital = candidate.get();
                if (!hospital) {
                    for (auto& candidate : engine_.hospitals()) {
                        if (!candidate->hasSpace()) continue;
                        if (!hospital || Dijkstra::manhattan(vehicle->position(), candidate->position()) <
                                         Dijkstra::manhattan(vehicle->position(), hospital->position()))
                            hospital = candidate.get();
                    }
                }
                if (!hospital) { setFeedback("Both hospitals are full."); break; }
                selectedHospitalId_ = hospital->id();
                if (!vehicle || !(vehicle->position() == hospital->position())) {
                    routeTo(hospital->position(), hospital->name());
                    setFeedback("Route planned to deliver " + patient->name() + ". Press Deliver on arrival.");
                    break;
                }
                const RescueResult result = engine_.deliverToHospital(patient->id(), hospital->id());
                setFeedback(result.message, result.success);
                if (result.success) audio_.playSuccess();
                break;
            }
            case 4: {
                Civilian* target = nullptr;
                for (auto& civilian : engine_.civilians())
                    if (civilian.id() == selectedCivilianId_ && !civilian.isRescued() &&
                        civilian.status() != CivilianStatus::Treated) target = &civilian;
                if (!target && vehicle)
                    for (auto& civilian : engine_.civilians())
                        if (civilian.position() == vehicle->position() && !civilian.isRescued() &&
                            civilian.status() != CivilianStatus::Treated) { target = &civilian; break; }
                if (!target) {
                    auto targets = engine_.recommendedTargets();
                    for (auto* candidate : targets)
                        if (candidate->status() != CivilianStatus::Treated) { target = candidate; break; }
                }
                if (!target) { setFeedback("No untreated civilian is available."); break; }
                selectedCivilianId_ = target->id();
                if (!vehicle || !(vehicle->position() == target->position())) {
                    routeTo(target->position(), target->name());
                    setFeedback("Route planned to treat " + target->name() + ". Press Treat on arrival.");
                    break;
                }
                const RescueResult result = engine_.treatCivilian(target->id());
                setFeedback(result.message, result.success);
                if (result.success) audio_.playSuccess();
                break;
            }
            case 5:
                if (engine_.refuelSelectedVehicle()) setFeedback(vehicle->name() + " refuelled to full.", true);
                else setFeedback("Move the selected vehicle to the Fire Station first.");
                break;
            case 6:
                engine_.setPaused(!engine_.paused());
                setFeedback(engine_.paused() ? "Mission paused." : "Mission resumed.", true);
                break;
            case 7: state_ = GameState::Visualizer; break;
            case 8:
                if (engine_.saveGame("saves/save.txt")) setFeedback("Mission saved to saves/save.txt.", true);
                else setFeedback("Could not write saves/save.txt.");
                break;
            case 9: state_ = GameState::MainMenu; break;
        }
        return;
    }

    Position cell;
    if (mapRenderer_->cellFromPixel(mouse, cell)) selectMapCell(cell);
}

void GameRenderer::handleVisualizerClick(sf::Vector2f mouse) {
    for (int mode = 0; mode < 3; ++mode) {
        if (!inRect(mouse, 300.f + static_cast<float>(mode) * 160.f, 675.f, 150.f, 42.f)) continue;
        engine_.setRouteMode(static_cast<RouteMode>(mode));
        if (hasSelectedDestination_) routeTo(selectedDestination_, "selected destination");
        else {
            auto targets = engine_.recommendedTargets();
            if (!targets.empty()) {
                selectedCivilianId_ = targets.front()->id();
                routeTo(targets.front()->position(), targets.front()->name());
            }
        }
        return;
    }
    if (inRect(mouse, 820.f, 675.f, 150.f, 42.f)) {
        state_ = GameState::Playing;
        return;
    }
    Position cell;
    if (mapRenderer_->cellFromPixel(mouse, cell)) selectMapCell(cell);
}

void GameRenderer::handleSettingsClick(sf::Vector2f mouse) {
    if (inRect(mouse, 500.f, 500.f, 280.f, 50.f)) {
        state_ = GameState::MainMenu;
        return;
    }
    if (inRect(mouse, 770.f, 145.f, 170.f, 48.f)) {
        settings_.soundOn = !settings_.soundOn;
        audio_.setEnabled(settings_.soundOn);
        if (saveSettings())
            setFeedback(std::string("Sound effects ") + (settings_.soundOn ? "enabled." : "disabled."), true);
        return;
    }
    if (inRect(mouse, 700.f, 225.f, 54.f, 46.f)) settings_.volume -= 10;
    else if (inRect(mouse, 850.f, 225.f, 54.f, 46.f)) settings_.volume += 10;
    else if (inRect(mouse, 700.f, 310.f, 54.f, 46.f)) settings_.animationSpeed -= 0.25;
    else if (inRect(mouse, 850.f, 310.f, 54.f, 46.f)) settings_.animationSpeed += 0.25;
    else return;

    settings_.volume = std::clamp(settings_.volume, 0, 100);
    settings_.animationSpeed = std::clamp(settings_.animationSpeed, 0.25, 3.0);
    audio_.setVolume(settings_.volume);
    if (saveSettings()) setFeedback("Settings updated and saved.", true);
}

void GameRenderer::run() {
    while (window_.isOpen()) {
        const float dt = clock_.restart().asSeconds();
        particles_.update(dt);
        if (feedbackSeconds_ > 0.f) feedbackSeconds_ = std::max(0.f, feedbackSeconds_ - dt);

        sf::Event event;
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window_.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                const sf::Vector2f mouse = window_.mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                if (state_ == GameState::MainMenu) handleMenuClick(mouse);
                else if (state_ == GameState::LevelSelect) {
                    for (int level = 0; level < 5; ++level) {
                        const float y = 135.f + static_cast<float>(level) * 68.f;
                        if (!inRect(mouse, 380.f, y, 520.f, 52.f)) continue;
                        engine_.newMission(level + 1);
                        selectedCivilianId_ = -1;
                        hasSelectedDestination_ = false;
                        state_ = GameState::Playing;
                        setFeedback("Level " + std::to_string(level + 1) + " mission started.", true);
                        break;
                    }
                    if (inRect(mouse, 380.f, 545.f, 200.f, 46.f)) state_ = GameState::MainMenu;
                } else if (state_ == GameState::Playing) handleDashboardClick(mouse);
                else if (state_ == GameState::Visualizer) handleVisualizerClick(mouse);
                else if (state_ == GameState::Help || state_ == GameState::Statistics) {
                    if (inRect(mouse, 500.f, state_ == GameState::Help ? 520.f : 650.f, 280.f, 50.f))
                        state_ = GameState::MainMenu;
                } else if (state_ == GameState::Settings) handleSettingsClick(mouse);
                else if (state_ == GameState::GameOver || state_ == GameState::Victory) {
                    if (inRect(mouse, 450.f, 330.f, 380.f, 50.f)) {
                        engine_.reset();
                        selectedCivilianId_ = -1;
                        hasSelectedDestination_ = false;
                        state_ = GameState::Playing;
                    } else if (inRect(mouse, 450.f, 400.f, 380.f, 50.f)) {
                        state_ = GameState::MainMenu;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    if (state_ != GameState::MainMenu) state_ = GameState::MainMenu;
                } else if (event.key.code == sf::Keyboard::Space && state_ == GameState::Playing) {
                    engine_.setPaused(!engine_.paused());
                    setFeedback(engine_.paused() ? "Mission paused." : "Mission resumed.", true);
                } else if (state_ == GameState::Playing &&
                           event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num4) {
                    const int id = static_cast<int>(event.key.code - sf::Keyboard::Num1) + 1;
                    engine_.selectVehicle(id);
                    setFeedback("Selected " + engine_.selectedVehicle()->name() + ".", true);
                }
            }
        }

        if (state_ == GameState::Playing && !engine_.paused()) {
            const float stepInterval = static_cast<float>(1.0 / settings_.animationSpeed);
            tickAccumulator_ += dt;
            while (tickAccumulator_ >= stepInterval) {
                tickAccumulator_ -= stepInterval;
                engine_.tick();
                engine_.moveVehicleAlongRoute();
                if (engine_.isVictory() || engine_.isDefeat()) break;
            }
            if (engine_.isVictory()) state_ = GameState::Victory;
            else if (engine_.isDefeat()) state_ = GameState::GameOver;
        }

        switch (state_) {
            case GameState::MainMenu: drawMainMenu(); break;
            case GameState::LevelSelect: drawLevelSelect(); break;
            case GameState::Playing: case GameState::Paused: drawDashboard(); break;
            case GameState::Visualizer: drawVisualizer(); break;
            case GameState::Help: drawHelp(); break;
            case GameState::Settings: drawSettings(); break;
            case GameState::Statistics: drawStatistics(); break;
            case GameState::GameOver: drawGameOver(false); break;
            case GameState::Victory: drawGameOver(true); break;
        }
        window_.display();
    }
}

} // namespace drc
#endif

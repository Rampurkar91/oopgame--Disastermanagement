#ifdef DRC_HAS_SFML
#include "../../include/ui/GameRenderer.hpp"
#include "../../include/managers/SaveManager.hpp"
#include <sstream>

namespace drc {

GameRenderer::GameRenderer() : window_(sf::VideoMode(1280, 800), "Disaster Rescue Commander") {
    window_.setFramerateLimit(60);
    loadFont();
    mapRenderer_ = std::make_unique<MapRenderer>(sf::Vector2f(300, 90), 44.0f);
}

bool GameRenderer::loadFont() {
    const char* candidates[] = {"assets/fonts/DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "DejaVuSans.ttf"};
    for (auto p : candidates) if (font_.loadFromFile(p)) return true;
    return false;
}

bool GameRenderer::inRect(sf::Vector2f p, float x, float y, float w, float h) const {
    return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
}
void GameRenderer::drawText(const std::string& s, float x, float y, unsigned size, sf::Color c) {
    sf::Text t; t.setFont(font_); t.setString(s); t.setCharacterSize(size); t.setFillColor(c);
    t.setPosition(x, y); window_.draw(t);
}
void GameRenderer::drawButton(sf::Vector2f pos, sf::Vector2f size, const std::string& label, bool selected) {
    sf::RectangleShape r(size); r.setPosition(pos);
    r.setFillColor(selected ? sf::Color(0,217,255) : sf::Color(22,119,255));
    r.setOutlineColor(sf::Color(0,217,255)); r.setOutlineThickness(2);
    window_.draw(r);
    drawText(label, pos.x + 12, pos.y + 8, 16, sf::Color::White);
}

void GameRenderer::drawMainMenu() {
    window_.clear(sf::Color(8, 20, 38));
    drawText("DISASTER RESCUE COMMANDER", 340, 60, 36, sf::Color(0,217,255));
    drawText("Intelligent City Disaster Simulation", 420, 110, 18, sf::Color(200,220,240));
    const char* items[] = {"Start New Mission","Continue Mission","Level Selection","How to Play","DSA Visualizer","Mission Statistics","Settings","Exit Game"};
    for (int i = 0; i < 8; ++i) drawButton({440, 170 + i * 62}, {400, 48}, items[i]);
    drawText("C++17 | SFML | Dijkstra Shortest Path | OOP Architecture", 380, 700, 13, sf::Color(120,150,180));
}

void GameRenderer::drawLevelSelect() {
    window_.clear(sf::Color(8, 20, 38));
    drawText("SELECT MISSION LEVEL", 420, 60, 30, sf::Color(0,217,255));
    const char* names[] = {"Level 1 - Beginner (Classic 9x13)","Level 2 - Intermediate","Level 3 - Advanced","Level 4 - Expert","Level 5 - Survival"};
    for (int i = 0; i < 5; ++i) drawButton({380, 140 + i * 70}, {520, 54}, names[i]);
    drawButton({380, 520}, {200, 48}, "Back");
}

void GameRenderer::drawDashboard() {
    window_.clear(sf::Color(8, 20, 38));
    // Top HUD
    auto* v = engine_.selectedVehicle();
    std::ostringstream hud;
    hud << engine_.statusSummary();
    if (v) hud << " | " << v->name() << " Fuel " << v->fuel() << "/" << v->maxFuel() << " Supplies " << v->supplies();
    drawText(hud.str(), 16, 10, 15, sf::Color::White);
    drawText(engine_.paused() ? "[PAUSED]" : "[RUNNING]", 16, 34, 14, sf::Color(255,212,59));
    // Left panel: civilians
    drawText("CIVILIANS", 16, 70, 16, sf::Color(0,217,255));
    float y = 95;
    for (auto& c : engine_.civilians()) {
        std::ostringstream s; s << c.name() << " H:" << c.health() << " P:" << c.priority() << " " << toString(c.condition()) << " " << toString(c.status());
        drawText(s.str(), 16, y, 12, c.condition() == HealthCondition::Critical ? sf::Color(240,68,82) : sf::Color(220,230,240));
        y += 20;
    }
    drawText("RECOMMENDED", 16, y + 8, 14, sf::Color(139,255,90));
    y += 30;
    for (auto* c : engine_.recommendedTargets()) { drawText(c->name() + " urgency " + std::to_string(c->urgencyScore()), 16, y, 12, sf::Color(220,230,240)); y += 18; }
    // Right panel: vehicle
    drawText("VEHICLE", 1020, 70, 16, sf::Color(0,217,255));
    if (v) {
        std::ostringstream s;
        s << v->name() << "\n" << v->description() << "\nPos " << v->position().toString()
          << "\nFuel " << v->fuel() << " (" << (int)v->fuelPercent() << "%)"
          << "\nSupplies " << v->supplies() << "\nStatus " << toString(v->status())
          << "\nRoute " << toString(engine_.routeMode()) << " dist " << engine_.lastRoute().distance;
        drawText(s.str(), 1020, 95, 13, sf::Color(220,230,240));
    }
    drawText("HOSPITALS", 1020, 300, 16, sf::Color(0,191,166));
    float hy = 325;
    for (auto& h : engine_.hospitals()) { drawText(h->name() + " " + std::to_string(h->occupied()) + "/" + std::to_string(h->capacity()), 1020, hy, 13, sf::Color(220,230,240)); hy += 20; }
    // Map
    mapRenderer_->draw(window_, engine_, font_);
    // Bottom action bar
    const char* acts[] = {"Select Vehicle","Find Route","Rescue","Deliver","Treat","Refuel","Pause","Visualizer","Save","Menu"};
    for (int i = 0; i < 10; ++i) drawButton({16 + i * 126, 730}, {118, 44}, acts[i]);
    // Legend
    drawText("Legend: blue=road grey=blocked cyan=flood red=fire teal=hospital amber=fire station", 300, 700, 11, sf::Color(150,180,200));
    particles_.draw(window_);
}

void GameRenderer::drawVisualizer() {
    window_.clear(sf::Color(8, 20, 38));
    drawText("DIJKSTRA VISUALIZER (real algorithm state)", 300, 20, 22, sf::Color(0,217,255));
    mapRenderer_->draw(window_, engine_, font_);
    auto r = engine_.lastRoute();
    std::ostringstream s;
    s << "Found: " << (r.found ? "yes" : "no") << "  Distance: " << r.distance << "  Steps: " << r.steps
      << "  Visited: " << r.visitedOrder.size() << "  Mode: " << toString(engine_.routeMode());
    drawText(s.str(), 300, 520, 14, sf::Color::White);
    drawText("Pseudocode: dist[]=INF; dist[start]=0; PQ.push(start); while PQ: u=PQ.popMin(); for v in neighbors(u): relax(u,v).", 300, 550, 12, sf::Color(180,200,220));
    drawText("Complexity: O((V+E) log V) time, O(V) space. V=117 cells.", 300, 575, 12, sf::Color(180,200,220));
    drawText("Visited order (first 20):", 300, 600, 12, sf::Color(139,255,90));
    std::string vo;
    for (size_t i = 0; i < r.visitedOrder.size() && i < 20; ++i) vo += r.visitedOrder[i].toString() + " ";
    drawText(vo, 300, 622, 11, sf::Color(220,230,240));
    const char* modes[] = {"FASTEST","SAFEST","LOW FUEL"};
    for (int i = 0; i < 3; ++i) drawButton({300 + i * 160, 660}, {150, 44}, modes[i], (int)engine_.routeMode() == i);
    drawButton({820, 660}, {150, 44}, "Back");
}

void GameRenderer::drawHelp() {
    window_.clear(sf::Color(8, 20, 38));
    drawText("HOW TO PLAY", 500, 40, 30, sf::Color(0,217,255));
    drawText("1. Select a vehicle (bottom bar).  2. Click a map cell to plan a route (Find Route).\n"
             "3. Move along the route, rescue civilians, deliver them to hospitals.\n"
             "4. Treat civilians with medical supplies (+20). Hospital admission +50.\n"
             "5. Refuel at the Fire Station (2,8). One fuel per cell moved.\n"
             "6. Ground vehicles cannot enter BLOCKED/FLOOD/FIRE. FireTruck may enter FIRE. Helicopter flies over all.\n"
             "Keys: Space=pause, 1-4 select vehicle, Esc=menu.", 200, 120, 15, sf::Color(220,230,240));
    drawButton({500, 500}, {280, 50}, "Back");
}

void GameRenderer::drawGameOver(bool victory) {
    window_.clear(sf::Color(8, 20, 38));
    drawText(victory ? "MISSION COMPLETE!" : "MISSION FAILED", 400, 200, 36, victory ? sf::Color(139,255,90) : sf::Color(240,68,82));
    drawText(engine_.statusSummary(), 450, 260, 18, sf::Color::White);
    drawButton({450, 340}, {380, 50}, "Play Again");
    drawButton({450, 410}, {380, 50}, "Main Menu");
}

void GameRenderer::handleMenuClick(sf::Vector2f m) {
    for (int i = 0; i < 8; ++i) {
        if (inRect(m, 440, 170 + i * 62, 400, 48)) {
            switch (i) {
                case 0: engine_.newMission(1); state_ = GameState::Playing; break;
                case 1: if (engine_.loadGame("saves/save.txt")) state_ = GameState::Playing; break;
                case 2: state_ = GameState::LevelSelect; break;
                case 3: state_ = GameState::Help; break;
                case 4: state_ = GameState::Visualizer; break;
                case 5: state_ = GameState::Statistics; break;
                case 6: state_ = GameState::Settings; break;
                case 7: window_.close(); break;
            }
            return;
        }
    }
}
void GameRenderer::handleDashboardClick(sf::Vector2f m) {
    // Bottom bar actions
    const char* acts[] = {"Select Vehicle","Find Route","Rescue","Deliver","Treat","Refuel","Pause","Visualizer","Save","Menu"};
    (void)acts;
    for (int i = 0; i < 10; ++i) {
        if (inRect(m, 16 + i * 126, 730, 118, 44)) {
            switch (i) {
                case 0: { int id = engine_.selectedVehicleId() % 4 + 1; engine_.selectVehicle(id); break; }
                case 1: { Position goal{4,6}; mapRenderer_->cellFromPixel(m, goal); engine_.planRoute(goal); break; }
                case 2: for (auto& c : engine_.civilians()) if (!c.isRescued()) { auto r = engine_.rescueCivilian(c.id()); if (r.success) particles_.emit({600,400}, 20, sf::Color(139,255,90)); break; } break;
                case 3: for (auto& c : engine_.civilians()) if (c.status() == CivilianStatus::Rescued) { engine_.deliverToHospital(c.id(), 1); break; } break;
                case 4: for (auto& c : engine_.civilians()) if (!c.isRescued()) { engine_.treatCivilian(c.id()); break; } break;
                case 5: engine_.refuelSelectedVehicle(); break;
                case 6: engine_.setPaused(!engine_.paused()); break;
                case 7: state_ = GameState::Visualizer; break;
                case 8: engine_.saveGame("saves/save.txt"); break;
                case 9: state_ = GameState::MainMenu; break;
            }
            return;
        }
    }
    // Map click: plan route to clicked cell.
    Position cell;
    if (mapRenderer_->cellFromPixel(m, cell)) engine_.planRoute(cell);
}
void GameRenderer::handleVisualizerClick(sf::Vector2f m) {
    for (int i = 0; i < 3; ++i) if (inRect(m, 300 + i * 160, 660, 150, 44)) {
        engine_.setRouteMode((RouteMode)i);
        if (auto* v = engine_.selectedVehicle()) engine_.planRoute(v->position());
        return;
    }
    if (inRect(m, 820, 660, 150, 44)) state_ = GameState::Playing;
}

void GameRenderer::run() {
    while (window_.isOpen()) {
        float dt = clock_.restart().asSeconds();
        particles_.update(dt);
        sf::Event ev;
        while (window_.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window_.close();
            if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f m = window_.mapPixelToCoords(ev.mouseButton.pos);
                if (state_ == GameState::MainMenu || state_ == GameState::Statistics || state_ == GameState::Settings) handleMenuClick(m);
                else if (state_ == GameState::LevelSelect) {
                    for (int i = 0; i < 5; ++i) if (inRect(m, 380, 140 + i * 70, 520, 54)) { engine_.newMission(i + 1); state_ = GameState::Playing; }
                    if (inRect(m, 380, 520, 200, 48)) state_ = GameState::MainMenu;
                }
                else if (state_ == GameState::Playing) handleDashboardClick(m);
                else if (state_ == GameState::Visualizer) handleVisualizerClick(m);
                else if (state_ == GameState::Help) { if (inRect(m, 500, 500, 280, 50)) state_ = GameState::MainMenu; }
                else if (state_ == GameState::GameOver || state_ == GameState::Victory) {
                    if (inRect(m, 450, 340, 380, 50)) { engine_.reset(); state_ = GameState::Playing; }
                    if (inRect(m, 450, 410, 380, 50)) state_ = GameState::MainMenu;
                }
            }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Space && state_ == GameState::Playing) engine_.setPaused(!engine_.paused());
                if (ev.key.code == sf::Keyboard::Escape) state_ = GameState::MainMenu;
                if (state_ == GameState::Playing && ev.key.code >= sf::Keyboard::Num1 && ev.key.code <= sf::Keyboard::Num4)
                    engine_.selectVehicle(ev.key.code - sf::Keyboard::Num1 + 1);
            }
        }
        // Simulation tick: 1 time unit per second.
        if (state_ == GameState::Playing && !engine_.paused()) {
            tickAccumulator_ += dt;
            if (tickAccumulator_ >= 1.0f) { tickAccumulator_ = 0; engine_.tick(); engine_.moveVehicleAlongRoute(); }
            if (engine_.isVictory()) state_ = GameState::Victory;
            else if (engine_.isDefeat()) state_ = GameState::GameOver;
        }
        window_.clear();
        switch (state_) {
            case GameState::MainMenu: case GameState::Statistics: case GameState::Settings: drawMainMenu(); break;
            case GameState::LevelSelect: drawLevelSelect(); break;
            case GameState::Playing: case GameState::Paused: drawDashboard(); break;
            case GameState::Visualizer: drawVisualizer(); break;
            case GameState::Help: drawHelp(); break;
            case GameState::Victory: drawGameOver(true); break;
            case GameState::GameOver: drawGameOver(false); break;
        }
        window_.display();
    }
}

} // namespace drc
#endif

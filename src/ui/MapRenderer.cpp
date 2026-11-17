#ifdef DRC_HAS_SFML
#include "../../include/ui/MapRenderer.hpp"
namespace drc {
MapRenderer::MapRenderer(sf::Vector2f origin, float cellSize) : origin_(origin), cellSize_(cellSize) {}
sf::Color MapRenderer::colorFor(CellType t) const {
    switch (t) {
        case CellType::Road: return sf::Color(30, 60, 90);
        case CellType::Blocked: return sf::Color(120, 120, 130);
        case CellType::Flood: return sf::Color(0, 150, 200);
        case CellType::Fire: return sf::Color(240, 68, 82);
        case CellType::House: return sf::Color(60, 80, 110);
        case CellType::Hospital: return sf::Color(0, 191, 166);
        case CellType::FireStation: return sf::Color(255, 159, 28);
    }
    return sf::Color::White;
}
bool MapRenderer::cellFromPixel(sf::Vector2f pixel, Position& out) const {
    float cs = cellSize_ * zoom_;
    int c = int((pixel.x - origin_.x) / cs);
    int r = int((pixel.y - origin_.y) / cs);
    if (r < 0 || r >= DisasterMap::ROWS || c < 0 || c >= DisasterMap::COLS) return false;
    out = {r, c};
    return true;
}
void MapRenderer::draw(sf::RenderWindow& w, GameEngine& engine, const sf::Font& font) {
    float cs = cellSize_ * zoom_;
    auto& map = engine.map();
    for (int r = 0; r < DisasterMap::ROWS; ++r) {
        for (int c = 0; c < DisasterMap::COLS; ++c) {
            sf::RectangleShape cell(sf::Vector2f(cs - 2, cs - 2));
            cell.setPosition(origin_.x + c * cs, origin_.y + r * cs);
            cell.setFillColor(colorFor(map.getCell({r, c})));
            w.draw(cell);
        }
    }
    // Route preview (glowing line).
    auto route = engine.lastRoute();
    if (route.found) {
        sf::Color rc = engine.routeMode() == RouteMode::Fastest ? sf::Color(0,217,255)
                       : engine.routeMode() == RouteMode::Safest ? sf::Color(139,255,90)
                       : sf::Color(255,94,53);
        for (size_t i = 1; i < route.path.size(); ++i) {
            sf::Vertex line[] = {
                sf::Vertex({origin_.x + route.path[i-1].col * cs + cs/2, origin_.y + route.path[i-1].row * cs + cs/2}, rc),
                sf::Vertex({origin_.x + route.path[i].col * cs + cs/2, origin_.y + route.path[i].row * cs + cs/2}, rc)
            };
            w.draw(line, 2, sf::Lines);
        }
    }
    // Civilians.
    for (auto& civ : engine.civilians()) {
        if (civ.isRescued()) continue;
        sf::CircleShape dot(8);
        dot.setPosition(origin_.x + civ.position().col * cs + cs/2 - 8, origin_.y + civ.position().row * cs + cs/2 - 8);
        dot.setFillColor(civ.condition() == HealthCondition::Critical ? sf::Color(240,68,82) : sf::Color(255,212,59));
        w.draw(dot);
        sf::Text t; t.setFont(font); t.setString(civ.name()); t.setCharacterSize(10);
        t.setPosition(dot.getPosition() + sf::Vector2f(0, -12));
        w.draw(t);
    }
    // Vehicles.
    for (auto& v : engine.vehicles()) {
        sf::RectangleShape box(sf::Vector2f(20, 14));
        box.setPosition(origin_.x + v->position().col * cs + cs/2 - 10, origin_.y + v->position().row * cs + cs/2 - 7);
        box.setFillColor(v->id() == engine.selectedVehicleId() ? sf::Color(0,217,255) : sf::Color(22,119,255));
        w.draw(box);
        sf::Text t; t.setFont(font); t.setString(v->name().substr(0,4)); t.setCharacterSize(9);
        t.setPosition(box.getPosition() + sf::Vector2f(0, -12));
        w.draw(t);
    }
}
} // namespace drc
#endif

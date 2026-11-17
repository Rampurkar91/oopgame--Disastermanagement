#ifdef DRC_HAS_SFML
#include "../../include/ui/MapRenderer.hpp"
#include "../../include/ui/Theme.hpp"
namespace drc {
MapRenderer::MapRenderer(sf::Vector2f origin, float cellSize) : origin_(origin), cellSize_(cellSize) {}
sf::Color MapRenderer::colorFor(CellType t) const {
    switch (t) {
        case CellType::Road: return theme::DeepBlueColor;
        case CellType::Blocked: return theme::RoyalVioletColor;
        case CellType::Flood: return theme::ElectricBlueColor;
        case CellType::Fire: return theme::EmergencyRedColor;
        case CellType::House: return theme::MidnightNavyColor;
        case CellType::Hospital: return theme::TealColor;
        case CellType::FireStation: return theme::AmberColor;
    }
    return theme::NeonCyanColor;
}
bool MapRenderer::cellFromPixel(sf::Vector2f pixel, Position& out) const {
    const float cs = cellSize_ * zoom_;
    const float width = cs * DisasterMap::COLS;
    const float height = cs * DisasterMap::ROWS;
    // Check pixel bounds before converting: int(-0.2) truncates to zero in C++.
    if (pixel.x < origin_.x || pixel.y < origin_.y ||
        pixel.x >= origin_.x + width || pixel.y >= origin_.y + height) return false;
    const int c = static_cast<int>((pixel.x - origin_.x) / cs);
    const int r = static_cast<int>((pixel.y - origin_.y) / cs);
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
        sf::Color rc = engine.routeMode() == RouteMode::Fastest ? theme::NeonCyanColor
                       : engine.routeMode() == RouteMode::Safest ? theme::NeonGreenColor
                       : theme::RescueOrangeColor;
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
        const sf::Color civilianColor = civ.condition() == HealthCondition::Critical ? theme::EmergencyRedColor
                                      : civ.condition() == HealthCondition::Injured ? theme::WarningYellowColor
                                      : theme::EmeraldColor;
        dot.setFillColor(civilianColor);
        w.draw(dot);
        if (!font.getInfo().family.empty()) {
            sf::Text t; t.setFont(font); t.setString(civ.name()); t.setCharacterSize(10);
            t.setPosition(dot.getPosition() + sf::Vector2f(0, -12));
            w.draw(t);
        }
    }
    // Vehicles.
    for (auto& v : engine.vehicles()) {
        sf::RectangleShape box(sf::Vector2f(20, 14));
        box.setPosition(origin_.x + v->position().col * cs + cs/2 - 10, origin_.y + v->position().row * cs + cs/2 - 7);
        box.setFillColor(v->id() == engine.selectedVehicleId() ? theme::NeonCyanColor : theme::ElectricBlueColor);
        w.draw(box);
        if (!font.getInfo().family.empty()) {
            sf::Text t; t.setFont(font); t.setString(v->name().substr(0,4)); t.setCharacterSize(9);
            t.setPosition(box.getPosition() + sf::Vector2f(0, -12));
            w.draw(t);
        }
    }
}
} // namespace drc
#endif

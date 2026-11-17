#pragma once
#ifdef DRC_HAS_SFML
#include <SFML/Graphics.hpp>
#include "../managers/GameEngine.hpp"
namespace drc {
class MapRenderer {
public:
    MapRenderer(sf::Vector2f origin, float cellSize);
    void draw(sf::RenderWindow& w, GameEngine& engine, const sf::Font& font);
    bool cellFromPixel(sf::Vector2f pixel, Position& out) const;
    void setZoom(float z) { zoom_ = z; }
    float zoom() const { return zoom_; }
private:
    sf::Vector2f origin_;
    float cellSize_;
    float zoom_ = 1.0f;
    sf::Color colorFor(CellType t) const;
};
} // namespace drc
#endif

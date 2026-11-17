#pragma once
#ifdef DRC_HAS_SFML
#include <SFML/Graphics.hpp>
#include "../managers/GameEngine.hpp"
#include "../core/GameState.hpp"
#include "MapRenderer.hpp"
#include "ParticleSystem.hpp"
#include <vector>
#include <memory>
namespace drc {
class GameRenderer {
public:
    GameRenderer();
    void run(); // main SFML loop: menu -> dashboard -> visualizer
private:
    sf::RenderWindow window_;
    sf::Font font_;
    GameEngine engine_;
    GameState state_ = GameState::MainMenu;
    std::unique_ptr<MapRenderer> mapRenderer_;
    ParticleSystem particles_;
    sf::Clock clock_;
    float tickAccumulator_ = 0.0f;
    bool loadFont();
    void drawMainMenu();
    void drawLevelSelect();
    void drawDashboard();
    void drawVisualizer();
    void drawHelp();
    void drawGameOver(bool victory);
    void handleMenuClick(sf::Vector2f mouse);
    void handleDashboardClick(sf::Vector2f mouse);
    void handleVisualizerClick(sf::Vector2f mouse);
    bool inRect(sf::Vector2f p, float x, float y, float w, float h) const;
    void drawButton(sf::Vector2f pos, sf::Vector2f size, const std::string& label, bool selected=false);
    void drawText(const std::string& s, float x, float y, unsigned size, sf::Color c);
};
} // namespace drc
#endif

#pragma once
#ifdef DRC_HAS_SFML
#include <SFML/Graphics.hpp>
#include "../managers/GameEngine.hpp"
#include "../managers/SaveManager.hpp"
#include "../core/GameState.hpp"
#include "AudioManager.hpp"
#include "MapRenderer.hpp"
#include "Theme.hpp"
#include "ParticleSystem.hpp"
#include <memory>
#include <string>

namespace drc {
class GameRenderer {
public:
    GameRenderer();
    void run();
private:
    sf::RenderWindow window_;
    sf::Font font_;
    GameEngine engine_;
    SettingsManager settings_;
    AudioManager audio_;
    GameState state_ = GameState::MainMenu;
    std::unique_ptr<MapRenderer> mapRenderer_;
    ParticleSystem particles_;
    sf::Clock clock_;
    float tickAccumulator_ = 0.0f;
    int selectedCivilianId_ = -1;
    int selectedHospitalId_ = 1;
    Position selectedDestination_{-1, -1};
    bool hasSelectedDestination_ = false;
    std::string feedback_;
    float feedbackSeconds_ = 0.0f;
    bool feedbackSuccess_ = false;

    bool loadFont();
    void drawMainMenu();
    void drawLevelSelect();
    void drawDashboard();
    void drawVisualizer();
    void drawHelp();
    void drawSettings();
    void drawStatistics();
    void drawGameOver(bool victory);
    void handleMenuClick(sf::Vector2f mouse);
    void handleDashboardClick(sf::Vector2f mouse);
    void handleVisualizerClick(sf::Vector2f mouse);
    void handleSettingsClick(sf::Vector2f mouse);
    void selectMapCell(Position cell);
    void routeTo(Position destination, const std::string& description);
    void setFeedback(const std::string& message, bool success = false);
    bool saveSettings();
    bool inRect(sf::Vector2f p, float x, float y, float w, float h) const;
    void drawButton(sf::Vector2f pos, sf::Vector2f size, const std::string& label, bool selected=false);
    void drawText(const std::string& s, float x, float y, unsigned size, sf::Color c);
};
} // namespace drc
#endif

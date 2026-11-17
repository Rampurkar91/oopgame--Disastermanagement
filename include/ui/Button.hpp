#pragma once
#ifdef DRC_HAS_SFML
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
namespace drc {
class Button {
public:
    Button(sf::Vector2f pos, sf::Vector2f size, std::string label, std::function<void()> onClick);
    void draw(sf::RenderWindow& w, const sf::Font& font) const;
    bool handleClick(sf::Vector2f mouse);
    bool contains(sf::Vector2f p) const;
    void setEnabled(bool e) { enabled_ = e; }
private:
    sf::RectangleShape shape_;
    std::string label_;
    std::function<void()> onClick_;
    bool enabled_ = true;
};
} // namespace drc
#endif

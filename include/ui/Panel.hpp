#pragma once
#ifdef DRC_HAS_SFML
#include <SFML/Graphics.hpp>
#include <string>
namespace drc {
class Panel {
public:
    Panel(sf::Vector2f pos, sf::Vector2f size, std::string title);
    void draw(sf::RenderWindow& w, const sf::Font& font) const;
    void setBody(std::string b) { body_ = std::move(b); }
private:
    sf::RectangleShape shape_;
    std::string title_;
    std::string body_;
};
} // namespace drc
#endif

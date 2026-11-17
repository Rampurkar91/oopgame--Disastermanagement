#ifdef DRC_HAS_SFML
#include "../../include/ui/Panel.hpp"
namespace drc {
Panel::Panel(sf::Vector2f pos, sf::Vector2f size, std::string title) : title_(std::move(title)) {
    shape_.setPosition(pos); shape_.setSize(size);
    shape_.setFillColor(sf::Color(16, 42, 67, 220));
    shape_.setOutlineColor(sf::Color(0, 217, 255, 120)); shape_.setOutlineThickness(1);
}
void Panel::draw(sf::RenderWindow& w, const sf::Font& font) const {
    w.draw(shape_);
    sf::Text t; t.setFont(font); t.setString(title_); t.setCharacterSize(16);
    t.setFillColor(sf::Color(0, 217, 255));
    t.setPosition(shape_.getPosition() + sf::Vector2f(10, 8));
    w.draw(t);
    sf::Text b; b.setFont(font); b.setString(body_); b.setCharacterSize(13);
    b.setFillColor(sf::Color(220, 230, 240));
    b.setPosition(shape_.getPosition() + sf::Vector2f(10, 34));
    w.draw(b);
}
} // namespace drc
#endif

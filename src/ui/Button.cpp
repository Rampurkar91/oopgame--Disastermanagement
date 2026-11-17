#ifdef DRC_HAS_SFML
#include "../../include/ui/Button.hpp"
namespace drc {
Button::Button(sf::Vector2f pos, sf::Vector2f size, std::string label, std::function<void()> onClick)
    : label_(std::move(label)), onClick_(std::move(onClick)) {
    shape_.setPosition(pos); shape_.setSize(size);
    shape_.setFillColor(sf::Color(22, 119, 255));
    shape_.setOutlineColor(sf::Color(0, 217, 255)); shape_.setOutlineThickness(2);
}
bool Button::contains(sf::Vector2f p) const { return shape_.getGlobalBounds().contains(p); }
bool Button::handleClick(sf::Vector2f mouse) {
    if (enabled_ && contains(mouse) && onClick_) { onClick_(); return true; }
    return false;
}
void Button::draw(sf::RenderWindow& w, const sf::Font& font) const {
    w.draw(shape_);
    sf::Text t; t.setFont(font); t.setString(label_); t.setCharacterSize(18);
    t.setFillColor(sf::Color::White);
    auto b = shape_.getGlobalBounds();
    t.setPosition(b.left + 12, b.top + (b.height - 24) / 2);
    w.draw(t);
}
} // namespace drc
#endif

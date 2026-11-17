#pragma once
#ifdef DRC_HAS_SFML
#include <SFML/Graphics.hpp>
#include <vector>
namespace drc {
struct Particle { sf::Vector2f pos; sf::Vector2f vel; float life = 1.0f; sf::Color color; };
class ParticleSystem {
public:
    void emit(sf::Vector2f pos, int count, sf::Color color);
    void update(float dt);
    void draw(sf::RenderWindow& w) const;
private:
    std::vector<Particle> particles_;
};
} // namespace drc
#endif

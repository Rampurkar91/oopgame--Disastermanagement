#ifdef DRC_HAS_SFML
#include "../../include/ui/ParticleSystem.hpp"
#include <random>
namespace drc {
void ParticleSystem::emit(sf::Vector2f pos, int count, sf::Color color) {
    static std::mt19937 rng(42);
    for (int i = 0; i < count; ++i) {
        Particle p; p.pos = pos; p.color = color; p.life = 1.0f;
        p.vel = sf::Vector2f((float)(rng() % 100 - 50) / 50.0f, (float)(rng() % 100 - 50) / 50.0f);
        particles_.push_back(p);
    }
}
void ParticleSystem::update(float dt) {
    for (auto& p : particles_) { p.pos += p.vel * dt * 60.0f; p.life -= dt; }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& p){ return p.life <= 0; }), particles_.end());
}
void ParticleSystem::draw(sf::RenderWindow& w) const {
    for (auto& p : particles_) {
        sf::CircleShape s(3); s.setPosition(p.pos); s.setFillColor(p.color); w.draw(s);
    }
}
} // namespace drc
#endif

#include "../../include/ui/AudioManager.hpp"
#ifdef DRC_HAS_SFML
#include <SFML/Audio.hpp>
#include <memory>
#include <unordered_map>
#endif

namespace drc {
void AudioManager::beep(const std::string& name) {
    if (!enabled_) return;
#ifdef DRC_HAS_SFML
    // Keep loaded buffers alive for as long as SFML's Sound may reference them.
    static std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers;
    static sf::Sound sound;
    auto it = buffers.find(name);
    if (it == buffers.end()) {
        auto buffer = std::make_unique<sf::SoundBuffer>();
        if (!buffer->loadFromFile("assets/sounds/" + name + ".wav")) return;
        it = buffers.emplace(name, std::move(buffer)).first;
    }
    sound.stop();
    sound.setBuffer(*it->second);
    sound.setVolume(static_cast<float>(volume_));
    sound.play();
#else
    (void)name;
#endif
}
void AudioManager::playClick() { beep("click"); }
void AudioManager::playRescue() { beep("rescue"); }
void AudioManager::playAlarm() { beep("alarm"); }
void AudioManager::playSuccess() { beep("success"); }
} // namespace drc

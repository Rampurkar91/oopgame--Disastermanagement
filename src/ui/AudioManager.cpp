#include "../../include/ui/AudioManager.hpp"
#ifdef DRC_HAS_SFML
#include <SFML/Audio.hpp>
#endif
namespace drc {
void AudioManager::beep(const std::string& name) {
    if (!enabled_) return;
#ifdef DRC_HAS_SFML
    // Optional sound files in assets/sounds/; silently skip if missing.
    sf::SoundBuffer buf;
    std::string path = "assets/sounds/" + name + ".wav";
    if (buf.loadFromFile(path)) {
        static sf::Sound sound;
        sound.setBuffer(buf);
        sound.setVolume((float)volume_);
        sound.play();
    }
#else
    (void)name;
#endif
}
void AudioManager::playClick() { beep("click"); }
void AudioManager::playRescue() { beep("rescue"); }
void AudioManager::playAlarm() { beep("alarm"); }
void AudioManager::playSuccess() { beep("success"); }
} // namespace drc

#include "../../include/managers/SaveManager.hpp"
#include <fstream>
#include <sstream>
namespace drc {
bool SaveManager::saveText(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) return false;
    f << content;
    return true;
}
bool SaveManager::loadText(const std::string& path, std::string& out) {
    std::ifstream f(path);
    if (!f) return false;
    std::ostringstream ss; ss << f.rdbuf();
    out = ss.str();
    return true;
}
std::string SaveManager::escape(const std::string& s) { return s; }
bool SettingsManager::load(const std::string& path) {
    std::string c; if (!SaveManager::loadText(path, c)) return false;
    std::istringstream in(c); std::string line;
    while (std::getline(in, line)) {
        std::istringstream ls(line); std::string k; ls >> k;
        if (k == "volume=") ls >> volume;
        else if (k == "musicVolume=") ls >> musicVolume;
        else if (k == "soundOn=") ls >> soundOn;
        else if (k == "musicOn=") ls >> musicOn;
        else if (k == "graphicsQuality=") ls >> graphicsQuality;
        else if (k == "animationSpeed=") ls >> animationSpeed;
    }
    return true;
}
bool SettingsManager::save(const std::string& path) const {
    std::ostringstream ss;
    ss << "volume=" << volume << "\nmusicVolume=" << musicVolume << "\nsoundOn=" << soundOn
       << "\nmusicOn=" << musicOn << "\ngraphicsQuality=" << graphicsQuality << "\nanimationSpeed=" << animationSpeed << "\n";
    return SaveManager::saveText(path, ss.str());
}
} // namespace drc

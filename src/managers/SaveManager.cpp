#include "../../include/managers/SaveManager.hpp"
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace drc {

bool SaveManager::saveText(const std::string& path, const std::string& content) {
    try {
        const std::filesystem::path filePath(path);
        if (!filePath.parent_path().empty())
            std::filesystem::create_directories(filePath.parent_path());
        std::ofstream file(filePath);
        if (!file) return false;
        file << content;
        return file.good();
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool SaveManager::loadText(const std::string& path, std::string& out) {
    std::ifstream file(path);
    if (!file) return false;
    std::ostringstream stream;
    stream << file.rdbuf();
    if (!file.good() && !file.eof()) return false;
    out = stream.str();
    return true;
}

std::string SaveManager::escape(const std::string& s) { return s; }

bool SettingsManager::load(const std::string& path) {
    std::string content;
    if (!SaveManager::loadText(path, content)) return false;

    SettingsManager parsed = *this;
    std::istringstream input(content);
    std::string line;
    try {
        while (std::getline(input, line)) {
            const auto separator = line.find('=');
            if (separator == std::string::npos) continue;
            const std::string key = line.substr(0, separator);
            const std::string value = line.substr(separator + 1);
            if (key == "volume") parsed.volume = std::stoi(value);
            else if (key == "musicVolume") parsed.musicVolume = std::stoi(value);
            else if (key == "soundOn") parsed.soundOn = std::stoi(value) != 0;
            else if (key == "musicOn") parsed.musicOn = std::stoi(value) != 0;
            else if (key == "graphicsQuality") parsed.graphicsQuality = std::stoi(value);
            else if (key == "animationSpeed") parsed.animationSpeed = std::stod(value);
        }
    } catch (const std::exception&) {
        return false;
    }

    parsed.volume = std::clamp(parsed.volume, 0, 100);
    parsed.musicVolume = std::clamp(parsed.musicVolume, 0, 100);
    parsed.graphicsQuality = std::clamp(parsed.graphicsQuality, 0, 2);
    parsed.animationSpeed = std::clamp(parsed.animationSpeed, 0.25, 3.0);
    *this = parsed;
    return true;
}

bool SettingsManager::save(const std::string& path) const {
    std::ostringstream content;
    content << "volume=" << std::clamp(volume, 0, 100)
            << "\nmusicVolume=" << std::clamp(musicVolume, 0, 100)
            << "\nsoundOn=" << soundOn
            << "\nmusicOn=" << musicOn
            << "\ngraphicsQuality=" << std::clamp(graphicsQuality, 0, 2)
            << "\nanimationSpeed=" << std::clamp(animationSpeed, 0.25, 3.0) << "\n";
    return SaveManager::saveText(path, content.str());
}

} // namespace drc

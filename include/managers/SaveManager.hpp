#pragma once
#include <string>
namespace drc {
class SaveManager {
public:
    static bool saveText(const std::string& path, const std::string& content);
    static bool loadText(const std::string& path, std::string& out);
    static std::string escape(const std::string& s);
};
class SettingsManager {
public:
    int volume = 70;
    int musicVolume = 50;
    bool soundOn = true;
    bool musicOn = true;
    int graphicsQuality = 1; // 0 low, 1 medium, 2 high
    double animationSpeed = 1.0;
    bool load(const std::string& path);
    bool save(const std::string& path) const;
};
} // namespace drc

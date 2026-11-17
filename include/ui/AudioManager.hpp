#pragma once
#include <algorithm>
#include <string>
namespace drc {
class AudioManager {
public:
    void setEnabled(bool e) { enabled_ = e; }
    void setVolume(int v) { volume_ = std::clamp(v, 0, 100); }
    void playClick();
    void playRescue();
    void playAlarm();
    void playSuccess();
    bool enabled() const { return enabled_; }
private:
    bool enabled_ = true;
    int volume_ = 70;
    void beep(const std::string& name);
};
} // namespace drc

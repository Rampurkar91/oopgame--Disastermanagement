#pragma once
#include "../core/Position.hpp"
#include "../core/GameState.hpp"
#include <string>

namespace drc {

enum class HealthCondition { Critical, Injured, Safe };

inline std::string toString(HealthCondition c) {
    switch (c) {
        case HealthCondition::Critical: return "Critical";
        case HealthCondition::Injured: return "Injured";
        case HealthCondition::Safe: return "Safe";
    }
    return "Safe";
}
inline HealthCondition conditionForHealth(int h) {
    if (h <= 30) return HealthCondition::Critical;
    if (h <= 60) return HealthCondition::Injured;
    return HealthCondition::Safe;
}

// Abstract base (preserved original class name).
class Person {
public:
    Person(int id, std::string name, Position pos, int health, int priority)
        : id_(id), name_(std::move(name)), position_(pos), health_(health), priority_(priority) {}
    virtual ~Person() = default;
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    Position position() const { return position_; }
    void setPosition(Position p) { position_ = p; }
    int health() const { return health_; }
    void setHealth(int h) { health_ = std::max(0, std::min(100, h)); }
    int priority() const { return priority_; }
    HealthCondition condition() const { return conditionForHealth(health_); }
protected:
    int id_;
    std::string name_;
    Position position_;
    int health_;
    int priority_;
};

class Civilian : public Person {
public:
    Civilian(int id, std::string name, Position pos, int health, int priority)
        : Person(id, std::move(name), pos, health, priority) {}
    CivilianStatus status() const { return status_; }
    void setStatus(CivilianStatus s) { status_ = s; }
    int assignedVehicleId() const { return assignedVehicleId_; }
    void setAssignedVehicleId(int v) { assignedVehicleId_ = v; }
    int hospitalId() const { return hospitalId_; }
    void setHospitalId(int h) { hospitalId_ = h; }
    bool isRescued() const { return status_ == CivilianStatus::Rescued || status_ == CivilianStatus::Admitted || status_ == CivilianStatus::Treated; }
    // Recommendation score: higher priority + lower health = more urgent.
    int urgencyScore() const { return priority_ * 10 + (100 - health_) / 5; }
private:
    CivilianStatus status_ = CivilianStatus::Waiting;
    int assignedVehicleId_ = -1;
    int hospitalId_ = -1;
};

} // namespace drc

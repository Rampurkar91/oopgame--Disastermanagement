#pragma once
#include <string>

namespace drc {

enum class GameState { MainMenu, LevelSelect, Playing, Paused, Visualizer, GameOver, Victory, Settings, Help, Statistics };
enum class VehicleStatus { Idle, Moving, Rescuing, Delivering, Refueling, OutOfFuel, Disabled };
enum class CivilianStatus { Waiting, Assigned, Rescued, Admitted, Treated };
enum class RouteMode { Fastest, Safest, LowFuel };

inline std::string toString(RouteMode m) {
    switch (m) {
        case RouteMode::Fastest: return "FASTEST";
        case RouteMode::Safest: return "SAFEST";
        case RouteMode::LowFuel: return "LOW_FUEL";
    }
    return "FASTEST";
}
inline std::string toString(CivilianStatus s) {
    switch (s) {
        case CivilianStatus::Waiting: return "Waiting";
        case CivilianStatus::Assigned: return "Assigned";
        case CivilianStatus::Rescued: return "Rescued";
        case CivilianStatus::Admitted: return "Admitted";
        case CivilianStatus::Treated: return "Treated";
    }
    return "Waiting";
}
inline std::string toString(VehicleStatus s) {
    switch (s) {
        case VehicleStatus::Idle: return "Idle";
        case VehicleStatus::Moving: return "Moving";
        case VehicleStatus::Rescuing: return "Rescuing";
        case VehicleStatus::Delivering: return "Delivering";
        case VehicleStatus::Refueling: return "Refueling";
        case VehicleStatus::OutOfFuel: return "OutOfFuel";
        case VehicleStatus::Disabled: return "Disabled";
    }
    return "Idle";
}

} // namespace drc

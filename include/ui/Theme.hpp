#pragma once
#include <cstdint>

#ifdef DRC_HAS_SFML
#include <SFML/Graphics/Color.hpp>
#endif

// The project's exact 12-colour palette. String forms are useful to docs/tools;
// SFML colours are exposed only to the optional graphical target.
namespace drc::theme {
inline constexpr const char* MidnightNavy = "#081426";
inline constexpr const char* DeepBlue = "#102A43";
inline constexpr const char* ElectricBlue = "#1677FF";
inline constexpr const char* NeonCyan = "#00D9FF";
inline constexpr const char* Teal = "#00BFA6";
inline constexpr const char* Emerald = "#16C784";
inline constexpr const char* NeonGreen = "#8BFF5A";
inline constexpr const char* WarningYellow = "#FFD43B";
inline constexpr const char* Amber = "#FF9F1C";
inline constexpr const char* RescueOrange = "#FF5E35";
inline constexpr const char* EmergencyRed = "#F04452";
inline constexpr const char* RoyalViolet = "#9B5DE5";

#ifdef DRC_HAS_SFML
inline const sf::Color MidnightNavyColor(8, 20, 38);
inline const sf::Color DeepBlueColor(16, 42, 67);
inline const sf::Color ElectricBlueColor(22, 119, 255);
inline const sf::Color NeonCyanColor(0, 217, 255);
inline const sf::Color TealColor(0, 191, 166);
inline const sf::Color EmeraldColor(22, 199, 132);
inline const sf::Color NeonGreenColor(139, 255, 90);
inline const sf::Color WarningYellowColor(255, 212, 59);
inline const sf::Color AmberColor(255, 159, 28);
inline const sf::Color RescueOrangeColor(255, 94, 53);
inline const sf::Color EmergencyRedColor(240, 68, 82);
inline const sf::Color RoyalVioletColor(155, 93, 229);
#endif
} // namespace drc::theme

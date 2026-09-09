#pragma once

#include "Platform.hpp"

#include <array>

namespace cavernbloom::testLevel {

inline constexpr std::array<Platform, 5> platforms{{
    {{0.0F, -260.0F}, {1040.0F, 40.0F}},   // Ground; top at -240.
    {{-240.0F, -160.0F}, {160.0F, 20.0F}}, // First step; top at -150.
    {{-370.0F, -70.0F}, {140.0F, 20.0F}},  // Second step; top at -60.
    {{260.0F, -180.0F}, {50.0F, 120.0F}},  // Side obstacle.
    {{120.0F, -100.0F}, {140.0F, 20.0F}}   // Overhead collision check.
}};

} // namespace cavernbloom::testLevel

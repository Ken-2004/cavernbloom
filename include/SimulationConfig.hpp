#pragma once

#include <glm/vec2.hpp>

namespace cavernbloom::simulation {

inline constexpr double fixedStepSeconds = 1.0 / 120.0;
inline constexpr double maxFrameDeltaSeconds = 0.25;

// World units and seconds; positions denote rectangle centers, with +Y up.
inline constexpr float moveSpeed = 280.0F;
inline constexpr float gravity = -1800.0F;
inline constexpr float jumpSpeed = 650.0F;
inline constexpr glm::vec2 playerSize{40.0F, 64.0F};
inline constexpr glm::vec2 playerSpawn{0.0F, 80.0F};

// Temporary infinite horizontal boundary, visualized across the current view.
inline constexpr float floorTop = -240.0F;
inline constexpr float floorThickness = 40.0F;

} // namespace cavernbloom::simulation

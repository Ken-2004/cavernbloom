#pragma once

#include "SimulationConfig.hpp"
#include <glm/vec2.hpp>
#include <cstddef>

namespace cavernbloom::encounters {

inline constexpr std::size_t enemyCount = 6;
inline constexpr std::size_t hazardCount = 4;
inline constexpr float enemySpeed = 72.0F;
inline constexpr glm::vec2 enemySize{20.0F, 24.0F};
inline constexpr glm::vec2 hazardSize{16.0F, 12.0F};
inline constexpr float hostWidth = 128.0F;
// Reserve a fully supported player footprint plus clearance at each edge.
inline constexpr float edgeReserve = simulation::playerSize.x + 6.0F;

} // namespace cavernbloom::encounters

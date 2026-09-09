#pragma once

#include "Platform.hpp"

#include <span>

namespace cavernbloom::collision {

struct MovementResult {
    glm::vec2 position;
    glm::vec2 velocity;
    bool grounded = false;
};

// Touching edges are contact, not overlap.
[[nodiscard]] bool overlaps(const Platform& first, const Platform& second) noexcept;

// Resolve X, then Y, against the nearest crossed face on each axis.
// Requires positive sizes, a nonnegative timestep, and a nonpenetrating start.
[[nodiscard]] MovementResult moveAndResolve(
    const glm::vec2& position, const glm::vec2& size, const glm::vec2& velocity,
    float deltaSeconds, std::span<const Platform> platforms) noexcept;

} // namespace cavernbloom::collision

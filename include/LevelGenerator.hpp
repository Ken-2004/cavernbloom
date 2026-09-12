#pragma once

#include "Platform.hpp"
#include "Bounds2D.hpp"
#include "Collectible.hpp"
#include "Enemy.hpp"
#include "Hazard.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cavernbloom {
namespace generation {

inline constexpr std::uint32_t developmentSeed = 20260909U;
inline constexpr std::size_t routePlatformCount = 28;
inline constexpr std::size_t collectibleCount = 8;
inline constexpr glm::vec2 collectibleSize{14.0F, 18.0F};
inline constexpr float collectibleClearance = 12.0F;
inline constexpr glm::vec2 goalSize{20.0F, 72.0F};
inline constexpr float goalClearance = 4.0F;
inline constexpr std::size_t maxCandidateAttempts = 32;
inline constexpr float leftBound = -600.0F;
inline constexpr float minimumTop = -260.0F;
inline constexpr float maximumTop = -100.0F;
inline constexpr float startWidth = 144.0F;
inline constexpr float startTop = -240.0F;
inline constexpr float platformThickness = 20.0F;
inline constexpr int minimumWidth = 48;
inline constexpr int maximumWidth = 64;
inline constexpr int minimumGap = 42;
inline constexpr int maximumGap = 52;
inline constexpr float spawnClearance = 24.0F;
// Restrict local height changes further to keep crowded arcs clear and playable.
inline constexpr double verticalStepFraction = 0.5;

} // namespace generation

struct GeneratedLevel {
    // Every platform is on the primary route, in progression order.
    std::vector<Platform> platforms;
    std::vector<Collectible> collectibles;
    std::vector<Enemy> enemies;
    std::vector<Hazard> hazards;
    Platform goalZone;
    glm::vec2 spawnPosition;
    Bounds2D bounds;
    std::size_t goalPlatformIndex = 0;
    std::uint32_t seed = 0;
    std::size_t candidateAttempts = 0;
    std::size_t fallbackCount = 0;
};

// Zero attempts selects checked fallback placements; larger-than-bound budgets are rejected.
[[nodiscard]] GeneratedLevel generateLevel(
    std::uint32_t seed, std::size_t attemptsPerPlatform = generation::maxCandidateAttempts);

} // namespace cavernbloom

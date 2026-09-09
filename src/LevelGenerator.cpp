#include "LevelGenerator.hpp"
#include "JumpReachability.hpp"

#include <cmath>
#include <random>
#include <stdexcept>

#include <glm/common.hpp>

namespace cavernbloom {
namespace {

int sampleInteger(std::mt19937& random, int minimum, int maximum)
{
    // Fixed one-draw mapping avoids standard-library-specific distribution algorithms.
    const auto range = static_cast<std::uint32_t>(maximum - minimum + 1);
    return minimum + static_cast<int>(random() % range);
}

Platform nextPlatform(const Platform& source, float width, float gap, float verticalStep)
{
    return {{source.position.x + source.size.x * 0.5F + gap + width * 0.5F,
             source.position.y + verticalStep}, {width, generation::platformThickness}};
}

} // namespace

GeneratedLevel generateLevel(std::uint32_t seed, std::size_t attemptsPerPlatform)
{
    if (attemptsPerPlatform > generation::maxCandidateAttempts) {
        throw std::invalid_argument("Level candidate budget exceeds the bounded maximum.");
    }
    std::mt19937 random(seed);
    const JumpReachability reachability;
    const double maximumStep = reachability.maximumJumpHeight() * simulation::reachabilityFactor
                               * generation::verticalStepFraction;
    const int candidateStepRange = static_cast<int>(std::ceil(reachability.maximumJumpHeight()));

    GeneratedLevel level;
    level.seed = seed;
    level.platforms.reserve(generation::routePlatformCount);
    level.platforms.push_back({
        {generation::leftBound + generation::startWidth * 0.5F,
         generation::startTop - generation::platformThickness * 0.5F},
        {generation::startWidth, generation::platformThickness}});
    level.spawnPosition = {level.platforms.front().position.x,
                          generation::startTop + simulation::playerSize.y * 0.5F + generation::spawnClearance};

    while (level.platforms.size() < generation::routePlatformCount) {
        const Platform& source = level.platforms.back();
        const auto valid = [&](const Platform& candidate) {
            const float top = candidate.position.y + candidate.size.y * 0.5F;
            return reachability.canReach(source, candidate)
                   && std::abs(candidate.position.y - source.position.y) <= maximumStep
                   && top >= generation::minimumTop && top <= generation::maximumTop;
        };

        bool accepted = false;
        for (std::size_t attempt = 0; attempt < attemptsPerPlatform; ++attempt) {
            ++level.candidateAttempts;
            const float width = static_cast<float>(sampleInteger(random, generation::minimumWidth, generation::maximumWidth));
            const float gap = static_cast<float>(sampleInteger(random, generation::minimumGap, generation::maximumGap));
            const float rise = static_cast<float>(sampleInteger(random, -candidateStepRange, candidateStepRange));
            const Platform candidate = nextPlatform(source, width, gap, rise);
            if (valid(candidate)) {
                level.platforms.push_back(candidate);
                accepted = true;
                break;
            }
        }
        if (!accepted) {
            const Platform fallback = nextPlatform(source, static_cast<float>(generation::minimumWidth),
                                                    static_cast<float>(generation::minimumGap), 0.0F);
            if (!valid(fallback)) {
                throw std::runtime_error("Cannot place a reachable platform within the level bounds.");
            }
            level.platforms.push_back(fallback);
            ++level.fallbackCount;
        }
    }
    level.goalPlatformIndex = level.platforms.size() - 1;
    level.bounds.minimum = level.platforms.front().position - level.platforms.front().size * 0.5F;
    level.bounds.maximum = level.platforms.front().position + level.platforms.front().size * 0.5F;
    for (const Platform& platform : level.platforms) {
        level.bounds.minimum = glm::min(level.bounds.minimum, platform.position - platform.size * 0.5F);
        level.bounds.maximum = glm::max(level.bounds.maximum, platform.position + platform.size * 0.5F);
    }
    return level;
}

} // namespace cavernbloom

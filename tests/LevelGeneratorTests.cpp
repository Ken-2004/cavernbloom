#include "LevelGenerator.hpp"
#include "JumpReachability.hpp"
#include "Player.hpp"
#include "Collision.hpp"

#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using namespace cavernbloom;
constexpr float step = static_cast<float>(simulation::fixedStepSeconds);

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

float top(const Platform& platform)
{
    return platform.position.y + platform.size.y * 0.5F;
}

bool sameLayout(const GeneratedLevel& first, const GeneratedLevel& second)
{
    if (first.platforms.size() != second.platforms.size() || first.spawnPosition != second.spawnPosition
        || first.goalPlatformIndex != second.goalPlatformIndex
        || first.bounds.minimum != second.bounds.minimum || first.bounds.maximum != second.bounds.maximum) {
        return false;
    }
    for (std::size_t index = 0; index < first.platforms.size(); ++index) {
        if (first.platforms[index].position != second.platforms[index].position
            || first.platforms[index].size != second.platforms[index].size) {
            return false;
        }
    }
    return true;
}

void validate(const GeneratedLevel& level)
{
    const JumpReachability model;
    require(level.platforms.size() == generation::routePlatformCount, "Wrong route count.");
    require(level.goalPlatformIndex == level.platforms.size() - 1, "Invalid goal metadata.");
    require(level.candidateAttempts <= (level.platforms.size() - 1) * generation::maxCandidateAttempts,
            "Candidate budget exceeded.");
    const auto& start = level.platforms.front();
    require(start.size.x == generation::startWidth, "Wide start missing.");
    const float worldWidth = level.bounds.maximum.x - level.bounds.minimum.x;
    require(worldWidth >= 2500.0F && worldWidth <= 4000.0F, "World is not substantially larger than one screen.");
    require(level.platforms.back().position.x - level.spawnPosition.x > 1280.0F,
            "Goal is not beyond the initial view.");
    Bounds2D expectedBounds{start.position - start.size * 0.5F, start.position + start.size * 0.5F};
    require(std::isfinite(level.spawnPosition.x) && std::isfinite(level.spawnPosition.y), "Invalid spawn.");
    require(level.spawnPosition.x - simulation::playerSize.x * 0.5F >= start.position.x - start.size.x * 0.5F
                && level.spawnPosition.x + simulation::playerSize.x * 0.5F <= start.position.x + start.size.x * 0.5F
                && level.spawnPosition.y - simulation::playerSize.y * 0.5F > top(start), "Spawn is not safely above start.");
    for (std::size_t index = 0; index < level.platforms.size(); ++index) {
        const auto& platform = level.platforms[index];
        require(std::isfinite(platform.position.x) && std::isfinite(platform.position.y)
                    && std::isfinite(platform.size.x) && std::isfinite(platform.size.y)
                    && platform.size.x > 0.0F && platform.size.y > 0.0F, "Invalid platform geometry.");
        const glm::vec2 minimum = platform.position - platform.size * 0.5F;
        const glm::vec2 maximum = platform.position + platform.size * 0.5F;
        expectedBounds.minimum.x = std::min(expectedBounds.minimum.x, minimum.x);
        expectedBounds.minimum.y = std::min(expectedBounds.minimum.y, minimum.y);
        expectedBounds.maximum.x = std::max(expectedBounds.maximum.x, maximum.x);
        expectedBounds.maximum.y = std::max(expectedBounds.maximum.y, maximum.y);
        require(minimum.x >= level.bounds.minimum.x && minimum.y >= level.bounds.minimum.y
                    && maximum.x <= level.bounds.maximum.x && maximum.y <= level.bounds.maximum.y
                    && minimum.x >= generation::leftBound
                    && top(platform) >= generation::minimumTop && top(platform) <= generation::maximumTop,
                "Generated bounds do not contain a platform or vertical constraints changed.");
        require(!collision::overlaps({level.spawnPosition, simulation::playerSize}, platform), "Spawn penetrates geometry.");
        if (index > 0) {
            const auto& previous = level.platforms[index - 1];
            require(platform.position.x - platform.size.x * 0.5F > previous.position.x + previous.size.x * 0.5F,
                    "Route does not progress through separated platforms.");
            require(model.canReach(previous, platform), "Unreachable primary-route transition.");
        }
        for (std::size_t other = 0; other < index; ++other) {
            require(!collision::overlaps(platform, level.platforms[other]), "Overlapping primary platforms.");
        }
    }
    require(level.bounds.minimum == expectedBounds.minimum && level.bounds.maximum == expectedBounds.maximum,
            "Level bounds do not exactly match platform extents.");
}

void simulateTransition(const GeneratedLevel& level, std::size_t targetIndex)
{
    const auto& source = level.platforms[targetIndex - 1];
    const auto& target = level.platforms[targetIndex];
    // Start fully supported at the departure edge. Use the actual Player and all
    // level colliders, so another platform cannot silently obstruct a modeled jump.
    Player player({source.position.x + source.size.x * 0.5F - simulation::playerSize.x * 0.5F,
                   top(source) + simulation::playerSize.y * 0.5F});
    player.update(step, 0, false, level.platforms);
    require(player.grounded(), "Simulation takeoff lacks support.");
    player.update(step, 0, true, level.platforms);
    for (int tick = 0; tick < 180 && !player.grounded(); ++tick) {
        // A simple ideal input: rise clear of the target, move right, then stop
        // near its center. No search, teleportation, or gameplay AI is involved.
        const bool clearTop = player.position().y - player.size().y * 0.5F >= top(target);
        const int direction = clearTop && player.position().x < target.position.x - simulation::moveSpeed * step * 0.5F ? 1 : 0;
        player.update(step, direction, false, level.platforms);
    }
    require(player.grounded() && std::abs(player.position().y - (top(target) + player.size().y * 0.5F)) < 0.001F
                && std::abs(player.position().x - target.position.x) < simulation::moveSpeed * step,
            "Analytic route failed the real fixed-step jump check.");
}

} // namespace

int main()
{
    try {
        std::size_t upward = 0;
        std::size_t downward = 0;
        std::size_t sameHeight = 0;
        std::size_t changedLayouts = 0;
        float minimumWidth = 4000.0F;
        float maximumWidth = 0.0F;
        double totalWidth = 0.0;
        GeneratedLevel previous;
        for (std::uint32_t seed = 0; seed < 1000; ++seed) {
            try {
                const auto level = generateLevel(seed);
                const auto repeated = generateLevel(seed);
                validate(level);
                const float width = level.bounds.maximum.x - level.bounds.minimum.x;
                minimumWidth = std::min(minimumWidth, width);
                maximumWidth = std::max(maximumWidth, width);
                totalWidth += width;
                require(level.seed == seed && repeated.seed == seed, "Seed metadata incorrect.");
                require(sameLayout(level, repeated) && level.candidateAttempts == repeated.candidateAttempts
                            && level.fallbackCount == repeated.fallbackCount, "Same seed changed output.");
                if (seed > 0 && !sameLayout(previous, level)) { ++changedLayouts; }
                for (std::size_t index = 1; index < level.platforms.size(); ++index) {
                    const float rise = top(level.platforms[index]) - top(level.platforms[index - 1]);
                    if (rise > 0.0F) { ++upward; }
                    else if (rise < 0.0F) { ++downward; }
                    else { ++sameHeight; }
                    simulateTransition(level, index);
                }
                previous = level;
            } catch (const std::exception& error) {
                throw std::runtime_error("Seed " + std::to_string(seed) + ": " + error.what());
            }
        }
        require(changedLayouts > 990 && upward > 0 && downward > 0 && sameHeight > 0,
                "Seed sweep lacks layout or height variation.");
        const auto fallback = generateLevel(42, 0);
        validate(fallback);
        require(fallback.candidateAttempts == 0 && fallback.fallbackCount == generation::routePlatformCount - 1,
                "Zero-budget fallback was not bounded.");
        validate(generateLevel(42, 1));
        validate(generateLevel(0xFFFFFFFFU));
        const auto development = generateLevel(generation::developmentSeed);
        validate(development);
        Player spawn(development.spawnPosition);
        for (int tick = 0; tick < 120; ++tick) { spawn.update(step, 0, false, development.platforms); }
        require(spawn.grounded(), "Generated spawn failed to land.");
        spawn.resetToSpawn();
        require(spawn.position() == development.spawnPosition && spawn.velocity() == glm::vec2(0.0F)
                    && !spawn.grounded(), "Generated spawn reset failed.");
        bool rejected = false;
        try { (void)generateLevel(0, generation::maxCandidateAttempts + 1); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Unbounded generation budget accepted.");
        std::cout << "Seeds 0..999 passed; " << upward << " upward, " << downward
                  << " downward, " << sameHeight << " same-height jumps passed real physics.\n";
        std::cout << "Bounded fallback, extreme seed, development spawn and reset passed.\n";
        std::cout << "World widths: min=" << minimumWidth << ", max=" << maximumWidth
                  << ", mean=" << totalWidth / 1000.0 << "; development="
                  << development.bounds.maximum.x - development.bounds.minimum.x << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

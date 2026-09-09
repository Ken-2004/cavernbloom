#include "Collision.hpp"
#include "Player.hpp"
#include "TestLevel.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

using cavernbloom::Platform;
using cavernbloom::Player;
namespace collision = cavernbloom::collision;
namespace simulation = cavernbloom::simulation;
constexpr float step = static_cast<float>(simulation::fixedStepSeconds);
constexpr glm::vec2 smallSize{2.0F, 2.0F};
constexpr std::array<Platform, 1> ground{{{{0.0F, -260.0F}, {120.0F, 40.0F}}}};
constexpr float restingY = -208.0F;

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(float first, float second)
{
    return std::abs(first - second) < 0.0001F;
}

void settle(Player& player, std::span<const Platform> platforms = ground)
{
    for (int tick = 0; tick < 120; ++tick) {
        player.update(step, 0, false, platforms);
    }
    require(player.grounded(), "Player failed to become grounded.");
}

void checkOverlap()
{
    const Platform body{{0.0F, 0.0F}, smallSize};
    require(collision::overlaps(body, {{1.0F, 1.0F}, smallSize}), "Overlapping boxes missed.");
    require(collision::overlaps(body, body), "Identical boxes missed.");
    require(collision::overlaps(body, {{0.0F, 0.0F}, {0.5F, 0.5F}}), "Contained box missed.");
    require(!collision::overlaps(body, {{3.0F, 0.0F}, smallSize}), "Separated boxes overlap.");
    require(!collision::overlaps(body, {{2.0F, 0.0F}, smallSize}), "Touching sides overlap.");
    require(!collision::overlaps(body, {{0.0F, 2.0F}, smallSize}), "Touching tops overlap.");
    require(!collision::overlaps(body, {{2.0F, 2.0F}, smallSize}), "Touching corners overlap.");
}

void checkLandingAndStability()
{
    Player player;
    for (int tick = 0; tick < 2400; ++tick) {
        player.update(step, 0, false, ground);
        require(player.position().y >= restingY, "Player sank into ground.");
        if (tick >= 120) {
            require(player.position().y == restingY && player.grounded()
                        && player.velocity().y == 0.0F, "Ground contact was unstable.");
        }
    }
    player.update(step, 0, true, ground);
    require(!player.grounded() && player.velocity().y > 0.0F
                && player.position().y > restingY, "Jump did not immediately leave ground.");

    const std::array<Platform, 1> fractional{{{{0.0F, -260.13F}, {120.0F, 39.27F}}}};
    Player fractionalPlayer;
    settle(fractionalPlayer, fractional);
    const float contactY = fractionalPlayer.position().y;
    for (int tick = 0; tick < 2400; ++tick) {
        fractionalPlayer.update(step, 0, false, fractional);
        require(fractionalPlayer.position().y == contactY && fractionalPlayer.grounded(),
                "Fractional platform contact drifted.");
    }
}

void checkWalkOff()
{
    Player player;
    settle(player);
    bool leftEdge = false;
    for (int tick = 0; tick < 60; ++tick) {
        player.update(step, 1, false, ground);
        if (player.position().x - player.size().x * 0.5F >= 60.0F) {
            require(!player.grounded() && player.velocity().y < 0.0F,
                    "Walking off the edge retained ground contact.");
            leftEdge = true;
        } else {
            require(player.grounded(), "Partial edge overlap lost ground contact.");
        }
    }
    require(leftEdge && player.position().y < restingY, "Player failed to walk off and fall.");
}

void checkUnderside()
{
    const std::array<Platform, 1> ceiling{{{{0.0F, 0.0F}, {10.0F, 2.0F}}}};
    const auto result = collision::moveAndResolve({0.0F, -4.0F}, smallSize,
                                                  {2.0F, 10.0F}, 0.5F, ceiling);
    require(result.position.y == -2.0F && result.velocity.y == 0.0F && !result.grounded,
            "Underside collision did not stop upward motion.");
    require(result.velocity.x == 2.0F, "Underside collision erased horizontal velocity.");

    const std::array<Platform, 2> room{{ground[0], {{0.0F, -100.0F}, {120.0F, 20.0F}}}};
    Player player;
    settle(player, ground);
    player.update(step, 0, true, room);
    bool hitCeiling = false;
    for (int tick = 0; tick < 60; ++tick) {
        player.update(step, 0, false, room);
        require(player.position().y + player.size().y * 0.5F <= -110.0F,
                "Player penetrated ceiling.");
        if (player.velocity().y == 0.0F && !player.grounded()) {
            hitCeiling = true;
        }
    }
    require(hitCeiling && player.grounded(), "Head bump did not lead to landing.");
}

void checkSides()
{
    const std::array<Platform, 1> wall{{{{0.0F, 0.0F}, {2.0F, 10.0F}}}};
    for (const float direction : std::array{-1.0F, 1.0F}) {
        const auto result = collision::moveAndResolve({-4.0F * direction, 0.0F}, smallSize,
                                                      {10.0F * direction, 2.0F}, 0.5F, wall);
        require(result.position.x == -2.0F * direction && result.velocity.x == 0.0F,
                "Side collision failed to block penetration.");
        require(result.velocity.y == 2.0F && result.position.y == 1.0F,
                "Side collision erased vertical motion.");
        require(!result.grounded, "Side contact incorrectly grounded the body.");
    }
}

void checkEdgeLandingAndSliding()
{
    const std::array<Platform, 1> platform{{{{0.0F, 0.0F}, {10.0F, 2.0F}}}};
    const auto edge = collision::moveAndResolve({5.9F, 4.0F}, smallSize,
                                                {0.0F, -10.0F}, 0.5F, platform);
    require(edge.grounded && edge.position.y == 2.0F, "Narrow edge landing failed.");
    const auto touching = collision::moveAndResolve({6.0F, 4.0F}, smallSize,
                                                    {0.0F, -10.0F}, 0.5F, platform);
    require(!touching.grounded && touching.position.y < 2.0F, "Zero-width edge contact landed.");
    const auto landing = collision::moveAndResolve({0.0F, 4.0F}, smallSize,
                                                   {2.0F, -10.0F}, 0.5F, platform);
    require(landing.grounded && landing.velocity.x == 2.0F && landing.position.x == 1.0F,
            "Vertical landing erased horizontal movement.");
    const auto sliding = collision::moveAndResolve(landing.position, smallSize,
                                                   {2.0F, -1.0F}, 0.5F, platform);
    require(sliding.position.x == 2.0F && sliding.grounded, "Surface movement stuck.");
    const auto stationary = collision::moveAndResolve(sliding.position, smallSize,
                                                      {0.0F, 0.0F}, step, platform);
    require(stationary.grounded, "Stationary contact lost support.");

    const std::array<Platform, 2> adjacent{{{{-5.0F, -7.13F}, {10.0F, 1.23F}},
                                          {{5.0F, -7.13F}, {10.0F, 1.23F}}}};
    const glm::vec2 fractionalSize{1.3F, 3.17F};
    const float contactY = adjacent[0].position.y + adjacent[0].size.y * 0.5F
                          + fractionalSize.y * 0.5F;
    glm::vec2 position{-4.0F, contactY};
    for (int tick = 0; tick < 120; ++tick) {
        const auto across = collision::moveAndResolve(position, fractionalSize,
                                                      {8.0F, -1.0F}, step, adjacent);
        require(across.velocity.x == 8.0F && across.position.y == contactY && across.grounded,
                "Adjacent fractional platforms caused sticking or sinking.");
        require(!collision::overlaps({across.position, fractionalSize}, adjacent[0])
                    && !collision::overlaps({across.position, fractionalSize}, adjacent[1]),
                "Resolved fractional contact was reported as penetration.");
        position = across.position;
    }
    require(near(position.x, 4.0F), "Movement across a platform seam was blocked.");
}

void checkMultiplePlatformsAndCrossings()
{
    std::array<Platform, 2> platforms{{{{0.0F, -10.0F}, {10.0F, 2.0F}},
                                      {{0.0F, 0.0F}, {10.0F, 2.0F}}}};
    const auto first = collision::moveAndResolve({0.0F, 10.0F}, smallSize,
                                                 {0.0F, -100.0F}, 1.0F, platforms);
    std::reverse(platforms.begin(), platforms.end());
    const auto reversed = collision::moveAndResolve({0.0F, 10.0F}, smallSize,
                                                    {0.0F, -100.0F}, 1.0F, platforms);
    require(first.position.y == 2.0F && first.grounded, "Nearest elevated top was skipped.");
    require(first.position == reversed.position && first.velocity == reversed.velocity,
            "Platform iteration order changed landing.");
    const auto upward = collision::moveAndResolve({0.0F, -20.0F}, smallSize,
                                                  {0.0F, 100.0F}, 1.0F, platforms);
    require(upward.position.y == -12.0F && upward.velocity.y == 0.0F,
            "Upward crossing skipped nearest underside.");
    const std::array<Platform, 2> walls{{{{4.0F, 0.0F}, {0.2F, 10.0F}},
                                        {{2.0F, 0.0F}, {0.2F, 10.0F}}}};
    const auto sideways = collision::moveAndResolve({0.0F, 0.0F}, smallSize,
                                                    {100.0F, 0.0F}, 1.0F, walls);
    require(near(sideways.position.x, 0.9F), "Horizontal crossing skipped thin nearest wall.");
}

void checkTestLevelAndReset()
{
    const auto& platforms = cavernbloom::testLevel::platforms;
    Player player;
    settle(player, platforms);
    require(player.position().y == restingY, "Test level spawn did not land on ground.");
    player.update(step, -1, true, platforms);
    for (int tick = 0; tick < 89; ++tick) {
        player.update(step, -1, false, platforms);
    }
    require(player.grounded() && near(player.position().y, -118.0F), "First step is unreachable.");
    player.update(step, -1, true, platforms);
    for (int tick = 0; tick < 59; ++tick) {
        player.update(step, -1, false, platforms);
    }
    for (int tick = 0; tick < 30; ++tick) {
        player.update(step, 0, false, platforms);
    }
    require(player.grounded() && near(player.position().y, -28.0F), "Second step is unreachable.");
    for (int tick = 0; tick < 240; ++tick) {
        player.update(step, -1, false, platforms);
    }
    require(player.position().y < simulation::fallResetY && !player.grounded(),
            "Finite level still behaves like an infinite floor.");
    player.resetToSpawn();
    require(player.position() == simulation::playerSpawn && player.velocity() == glm::vec2(0.0F)
                && !player.grounded(), "Reset did not clear player motion and contact.");
}

void checkDeterminism()
{
    Player first;
    Player second;
    for (int tick = 0; tick < 3000; ++tick) {
        const int direction = (tick / 150) % 3 - 1;
        const bool jump = tick % 97 == 0;
        first.update(step, direction, jump, cavernbloom::testLevel::platforms);
        second.update(step, direction, jump, cavernbloom::testLevel::platforms);
        if (first.position().y < simulation::fallResetY) {
            first.resetToSpawn();
        }
        if (second.position().y < simulation::fallResetY) {
            second.resetToSpawn();
        }
        require(first.position() == second.position() && first.velocity() == second.velocity()
                    && first.grounded() == second.grounded(), "Identical simulations diverged.");
    }
}

} // namespace

int main()
{
    try {
        checkOverlap();
        checkLandingAndStability();
        checkWalkOff();
        checkUnderside();
        checkSides();
        checkEdgeLandingAndSliding();
        checkMultiplePlatformsAndCrossings();
        checkTestLevelAndReset();
        checkDeterminism();
        std::cout << "Nine collision groups passed, including level traversal and reset.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

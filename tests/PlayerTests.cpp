#include "Player.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

using cavernbloom::Player;
namespace simulation = cavernbloom::simulation;
constexpr float step = static_cast<float>(simulation::fixedStepSeconds);
constexpr float restingY = simulation::floorTop + simulation::playerSize.y * 0.5F;

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void settle(Player& player)
{
    for (int tick = 0; tick < 120; ++tick) {
        player.update(step, 0, false);
        require(player.position().y >= restingY, "Player penetrated the floor.");
    }
    require(player.position().y == restingY, "Player did not land.");
}

void checkMovement()
{
    Player player;
    for (int tick = 0; tick < 120; ++tick) {
        player.update(step, 1, false);
    }
    require(std::abs(player.position().x - simulation::moveSpeed) < 0.01F,
            "One second of movement has incorrect distance.");
    const float stoppedX = player.position().x;
    for (int tick = 0; tick < 30; ++tick) {
        player.update(step, 0, false);
    }
    require(player.position().x == stoppedX, "Horizontal movement did not stop.");
    for (int tick = 0; tick < 120; ++tick) {
        player.update(step, -1, false);
    }
    require(std::abs(player.position().x) < 0.01F, "Left/right speeds differ.");
}

void checkJumping()
{
    Player player;
    player.update(step, 0, true);
    require(player.position().y < simulation::playerSpawn.y, "Airborne spawn allowed a jump.");
    settle(player);
    Player baseline = player;
    player.update(step, 0, true);
    baseline.update(step, 0, true);
    require(player.position().y > restingY, "Grounded jump failed.");
    float peak = player.position().y;
    int airborneTicks = 1;
    while (player.position().y > restingY && airborneTicks < 120) {
        // Repeated fresh requests in midair must not change the trajectory.
        player.update(step, 0, true);
        baseline.update(step, 0, false);
        require(player.position() == baseline.position(), "Airborne request changed jump trajectory.");
        require(player.position().y >= restingY, "Jump fell through the floor.");
        peak = std::max(peak, player.position().y);
        ++airborneTicks;
    }
    require(player.position().y == restingY, "Jump did not return to the floor.");
    require(peak - restingY > 110.0F && peak - restingY < 120.0F, "Unexpected jump height.");
    require(airborneTicks >= 83 && airborneTicks <= 89, "Unexpected jump duration.");
    settle(player);
    player.update(step, 0, true);
    require(player.position().y > restingY, "Player cannot jump after landing.");
    std::cout << "Jump height: " << peak - restingY
              << " units; airtime: " << static_cast<float>(airborneTicks) * step << " s\n";
}

void checkFrameSchedules()
{
    // Drive the real Player with fixed steps grouped into different display cadences.
    for (const int framesPerSecond : std::array{30, 60, 144, 240}) {
        Player player;
        settle(player);
        double accumulator = 0.0;
        bool jumpPending = true;
        for (int frame = 0; frame < framesPerSecond * 2; ++frame) {
            accumulator += 1.0 / static_cast<double>(framesPerSecond);
            while (accumulator >= simulation::fixedStepSeconds) {
                player.update(step, 1, jumpPending);
                jumpPending = false;
                accumulator -= simulation::fixedStepSeconds;
            }
        }
        // Floating-point accumulation may leave the final tick just below its boundary.
        require(std::abs(player.position().x - 2.0F * simulation::moveSpeed)
                    <= simulation::moveSpeed * step + 0.01F,
                "Movement distance depends on display cadence.");
        require(player.position().y == restingY, "Jump did not land at this display cadence.");
        std::cout << framesPerSecond << " FPS schedule: x=" << player.position().x << '\n';
    }
}

} // namespace

int main()
{
    try {
        checkMovement();
        checkJumping();
        checkFrameSchedules();
        std::cout << "Player physics checks passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

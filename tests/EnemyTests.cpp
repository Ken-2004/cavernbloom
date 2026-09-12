#include "EnemySystem.hpp"
#include "EncounterConfig.hpp"
#include "Camera2D.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using namespace cavernbloom;
void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}
} // namespace

int main()
{
    try {
        const std::array<Enemy, 1> layout{{{{0.0F, 12.0F}, encounters::enemySize, -2.0F, 2.0F, 120.0F, 1, 1}}};
        EnemySystem system(layout);
        system.update();
        require(system.enemies()[0].position.x == 1.0F && system.enemies()[0].direction == 1, "Right movement failed.");
        system.update();
        require(system.enemies()[0].position.x == 2.0F && system.enemies()[0].direction == -1, "Right reversal failed.");
        system.update();
        require(system.enemies()[0].position.x == 1.0F && system.enemies()[0].direction == -1, "Right edge flipped twice.");
        for (int tick = 0; tick < 3; ++tick) { system.update(); }
        require(system.enemies()[0].position.x == -2.0F && system.enemies()[0].direction == 1, "Left reversal failed.");
        system.update();
        require(system.enemies()[0].position.x == -1.0F && system.enemies()[0].direction == 1, "Left edge flipped twice.");
        system.reset();
        require(system.enemies()[0].position == layout[0].position && system.enemies()[0].direction == layout[0].direction,
                "Reset did not restore generated state.");
        EnemySystem repeated(layout);
        Camera2D camera;
        camera.setWorldBounds({{-1000.0F, -300.0F}, {3000.0F, 0.0F}});
        for (int tick = 0; tick < 100000; ++tick) {
            camera.setViewportSize(600 + tick % 1000, 720);
            camera.follow({static_cast<float>(tick % 2500), 0.0F});
            system.update();
            repeated.update();
            const Enemy& enemy = system.enemies()[0];
            require(enemy.position == repeated.enemies()[0].position && enemy.direction == repeated.enemies()[0].direction,
                    "Camera-independent identical updates diverged.");
            require(std::isfinite(enemy.position.x) && std::isfinite(enemy.position.y)
                        && enemy.position.x >= enemy.patrolMinX && enemy.position.x <= enemy.patrolMaxX
                        && enemy.position.y == layout[0].position.y && enemy.size.x > 0.0F && enemy.size.y > 0.0F,
                    "Stress sequence left bounds or corrupted geometry.");
        }
        auto special = layout;
        special[0].patrolMinX = special[0].patrolMaxX = 0.0F;
        EnemySystem stationary(special);
        for (int tick = 0; tick < 100; ++tick) { stationary.update(); }
        require(stationary.enemies()[0].position.x == 0.0F && stationary.enemies()[0].direction == 1,
                "Zero-width interval oscillated direction.");
        special = layout;
        special[0].speed = 10000.0F;
        EnemySystem overshoot(special);
        overshoot.update();
        require(overshoot.enemies()[0].position.x == 2.0F && overshoot.enemies()[0].direction == -1, "Overshoot escaped boundary.");
        overshoot.update();
        require(overshoot.enemies()[0].position.x == -2.0F && overshoot.enemies()[0].direction == 1, "Large inward step failed.");
        for (int invalid = 0; invalid < 6; ++invalid) {
            special = layout;
            if (invalid == 0) { special[0].speed = std::numeric_limits<float>::infinity(); }
            if (invalid == 1) { special[0].size.x = 0.0F; }
            if (invalid == 2) { special[0].direction = 0; }
            if (invalid == 3) { special[0].patrolMinX = 3.0F; }
            if (invalid == 4) { special[0].position.x = 100.0F; }
            if (invalid == 5) { special[0].position.y = std::numeric_limits<float>::quiet_NaN(); }
            bool rejected = false;
            try { EnemySystem bad(special); } catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "Invalid patrol layout accepted.");
        }
        std::cout << "Patrol movement, both reversals, 100,000 bounded deterministic ticks, camera independence, reset and invalid inputs passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

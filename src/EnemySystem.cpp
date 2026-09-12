#include "EnemySystem.hpp"
#include "SimulationConfig.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cavernbloom {

EnemySystem::EnemySystem(std::span<const Enemy> layout)
    : initial_(layout.begin(), layout.end()), enemies_(initial_)
{
    for (const Enemy& enemy : enemies_) {
        if (!std::isfinite(enemy.position.x) || !std::isfinite(enemy.position.y)
            || !std::isfinite(enemy.size.x) || !std::isfinite(enemy.size.y)
            || enemy.size.x <= 0.0F || enemy.size.y <= 0.0F
            || !std::isfinite(enemy.patrolMinX) || !std::isfinite(enemy.patrolMaxX)
            || enemy.patrolMinX > enemy.patrolMaxX
            || enemy.position.x < enemy.patrolMinX || enemy.position.x > enemy.patrolMaxX
            || !std::isfinite(enemy.speed) || enemy.speed <= 0.0F
            || (enemy.direction != -1 && enemy.direction != 1)) {
            throw std::invalid_argument("Enemy layout requires finite positive geometry/speed and a valid patrol interval.");
        }
    }
}

void EnemySystem::update() noexcept
{
    for (Enemy& enemy : enemies_) {
        if (enemy.patrolMinX == enemy.patrolMaxX) {
            continue;
        }
        const double next = static_cast<double>(enemy.position.x)
            + static_cast<double>(enemy.direction) * enemy.speed * simulation::fixedStepSeconds;
        // Clamp at the reached edge and face inward once. Discard overshoot for this tick.
        if (next >= enemy.patrolMaxX) {
            enemy.position.x = enemy.patrolMaxX;
            enemy.direction = -1;
        } else if (next <= enemy.patrolMinX) {
            enemy.position.x = enemy.patrolMinX;
            enemy.direction = 1;
        } else {
            enemy.position.x = static_cast<float>(next);
        }
    }
}

void EnemySystem::reset() noexcept
{
    std::copy(initial_.begin(), initial_.end(), enemies_.begin());
}

} // namespace cavernbloom

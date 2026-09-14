#include "Gameplay.hpp"
#include "Collision.hpp"

namespace cavernbloom {

GameplayEvent detectContact(const Platform& playerBounds,
    std::span<const Enemy> enemies, std::span<const Hazard> hazards) noexcept
{
    for (const Enemy& enemy : enemies) {
        if (collision::overlaps(playerBounds, {enemy.position, enemy.size})) {
            return GameplayEvent::EnemyContact;
        }
    }
    for (const Hazard& hazard : hazards) {
        if (collision::overlaps(playerBounds, {hazard.position, hazard.size})) {
            return GameplayEvent::HazardContact;
        }
    }
    return GameplayEvent::None;
}

Gameplay::Gameplay(const GeneratedLevel& level)
    : level_(level), player_(level.spawnPosition), progression_(level.collectibles, level.goalZone),
      enemies_(level.enemies)
{
}

GameplayEvent Gameplay::update(int horizontalDirection, bool jumpPressed) noexcept
{
    transitions_ = {};
    if (progression_.state() == GameState::Won) {
        return GameplayEvent::None;
    }
    enemies_.update();
    transitions_.jumped = player_.update(static_cast<float>(simulation::fixedStepSeconds),
        horizontalDirection, jumpPressed, level_.platforms);
    const GameplayEvent failure = player_.position().y < simulation::fallResetY
        ? GameplayEvent::Fell : detectContact({player_.position(), player_.size()}, enemies_.enemies(), level_.hazards);
    if (failure != GameplayEvent::None) {
        respawn();
        transitions_.outcome = failure;
        return failure;
    }
    const std::size_t previousCount = progression_.collectedCount();
    transitions_.outcome = progression_.update({player_.position(), player_.size()})
        ? GameplayEvent::Won : GameplayEvent::None;
    transitions_.flowersCollected = progression_.collectedCount() - previousCount;
    return transitions_.outcome;
}

void Gameplay::respawn() noexcept
{
    player_.resetToSpawn();
    enemies_.reset();
}

void Gameplay::restart() noexcept
{
    transitions_ = {};
    respawn();
    progression_.reset();
}

} // namespace cavernbloom

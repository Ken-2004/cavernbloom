#include "Player.hpp"
#include "Collision.hpp"

#include <algorithm>

namespace cavernbloom {

Player::Player(glm::vec2 spawnPosition) noexcept
    : spawnPosition_(spawnPosition), position_(spawnPosition)
{
}

bool Player::update(float deltaSeconds, int horizontalDirection, bool jumpPressed,
                    std::span<const Platform> platforms) noexcept
{
    velocity_.x = static_cast<float>(std::clamp(horizontalDirection, -1, 1)) * simulation::moveSpeed;
    const bool jumped = jumpPressed && grounded_;
    if (jumped) {
        velocity_.y = simulation::jumpSpeed;
        grounded_ = false;
    }

    velocity_.y += simulation::gravity * deltaSeconds;
    const collision::MovementResult resolved = collision::moveAndResolve(
        position_, size_, velocity_, deltaSeconds, platforms);
    position_ = resolved.position;
    velocity_ = resolved.velocity;
    grounded_ = resolved.grounded;
    return jumped;
}

void Player::resetToSpawn() noexcept
{
    position_ = spawnPosition_;
    velocity_ = glm::vec2(0.0F);
    grounded_ = false;
}

} // namespace cavernbloom

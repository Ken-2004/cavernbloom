#include "Player.hpp"
#include "Collision.hpp"

#include <algorithm>

namespace cavernbloom {

void Player::update(float deltaSeconds, int horizontalDirection, bool jumpPressed,
                    std::span<const Platform> platforms) noexcept
{
    velocity_.x = static_cast<float>(std::clamp(horizontalDirection, -1, 1)) * simulation::moveSpeed;
    if (jumpPressed && grounded_) {
        velocity_.y = simulation::jumpSpeed;
        grounded_ = false;
    }

    velocity_.y += simulation::gravity * deltaSeconds;
    const collision::MovementResult resolved = collision::moveAndResolve(
        position_, size_, velocity_, deltaSeconds, platforms);
    position_ = resolved.position;
    velocity_ = resolved.velocity;
    grounded_ = resolved.grounded;
}

void Player::resetToSpawn() noexcept
{
    position_ = simulation::playerSpawn;
    velocity_ = glm::vec2(0.0F);
    grounded_ = false;
}

} // namespace cavernbloom

#include "Player.hpp"

#include <algorithm>

namespace cavernbloom {

void Player::update(float deltaSeconds, int horizontalDirection, bool jumpPressed) noexcept
{
    velocity_.x = static_cast<float>(std::clamp(horizontalDirection, -1, 1)) * simulation::moveSpeed;
    if (jumpPressed && grounded_) {
        velocity_.y = simulation::jumpSpeed;
        grounded_ = false;
    }

    velocity_.y += simulation::gravity * deltaSeconds;
    position_ += velocity_ * deltaSeconds;

    const float restingCenterY = simulation::floorTop + size_.y * 0.5F;
    if (position_.y <= restingCenterY) {
        position_.y = restingCenterY;
        velocity_.y = 0.0F;
        grounded_ = true;
    } else {
        grounded_ = false;
    }
}

} // namespace cavernbloom

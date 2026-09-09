#include "Camera2D.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

namespace cavernbloom {

void Camera2D::setWorldBounds(const Bounds2D& bounds)
{
    if (!std::isfinite(bounds.minimum.x) || !std::isfinite(bounds.minimum.y)
        || !std::isfinite(bounds.maximum.x) || !std::isfinite(bounds.maximum.y)
        || bounds.minimum.x > bounds.maximum.x || bounds.minimum.y > bounds.maximum.y
        || !std::isfinite(bounds.maximum.x - bounds.minimum.x)) {
        throw std::invalid_argument("Camera bounds must be finite and ordered.");
    }
    bounds_ = bounds;
    clampHorizontal();
}

void Camera2D::setViewportSize(int framebufferWidth, int framebufferHeight) noexcept
{
    if (framebufferWidth <= 0 || framebufferHeight <= 0) {
        return;
    }
    visibleSize_.x = visibleHeight * static_cast<float>(framebufferWidth)
                     / static_cast<float>(framebufferHeight);
    clampHorizontal();
}

void Camera2D::follow(const glm::vec2& target) noexcept
{
    if (!std::isfinite(target.x) || !std::isfinite(target.y)) {
        return;
    }
    // Shrink the dead zone for very narrow views so the target can still be followed.
    const float halfZone = std::min(horizontalDeadZoneHalfWidth, visibleSize_.x * 0.25F);
    if (target.x > center_.x + halfZone) {
        center_.x = target.x - halfZone;
    } else if (target.x < center_.x - halfZone) {
        center_.x = target.x + halfZone;
    }
    clampHorizontal();
}

void Camera2D::recenter(const glm::vec2& target) noexcept
{
    if (!std::isfinite(target.x) || !std::isfinite(target.y)) {
        return;
    }
    center_.x = target.x;
    clampHorizontal();
}

void Camera2D::clampHorizontal() noexcept
{
    const float width = bounds_.maximum.x - bounds_.minimum.x;
    if (width <= visibleSize_.x) {
        // A wider viewport necessarily shows empty space; balance it on both sides.
        center_.x = bounds_.minimum.x + width * 0.5F;
    } else {
        const float halfWidth = visibleSize_.x * 0.5F;
        center_.x = std::clamp(center_.x, bounds_.minimum.x + halfWidth, bounds_.maximum.x - halfWidth);
    }
}

glm::mat4 Camera2D::viewProjection() const noexcept
{
    const glm::vec2 half = visibleSize_ * 0.5F;
    const glm::mat4 projection = glm::ortho(-half.x, half.x, -half.y, half.y, -1.0F, 1.0F);
    const glm::mat4 view = glm::translate(glm::mat4(1.0F), glm::vec3(-center_, 0.0F));
    return projection * view;
}

} // namespace cavernbloom

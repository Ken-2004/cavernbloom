#pragma once

#include "Bounds2D.hpp"

#include <glm/mat4x4.hpp>

namespace cavernbloom {

class Camera2D final {
public:
    static constexpr float visibleHeight = 720.0F;
    static constexpr float horizontalDeadZoneHalfWidth = 120.0F;

    void setWorldBounds(const Bounds2D& bounds);
    // Framebuffer pixels determine aspect ratio; minimized/invalid sizes retain the last valid view.
    void setViewportSize(int framebufferWidth, int framebufferHeight) noexcept;
    void follow(const glm::vec2& target) noexcept;
    void recenter(const glm::vec2& target) noexcept;
    [[nodiscard]] glm::mat4 viewProjection() const noexcept;
    [[nodiscard]] const glm::vec2& center() const noexcept { return center_; }
    [[nodiscard]] const glm::vec2& visibleSize() const noexcept { return visibleSize_; }

private:
    void clampHorizontal() noexcept;

    glm::vec2 center_{0.0F};
    glm::vec2 visibleSize_{1280.0F, visibleHeight};
    Bounds2D bounds_;
};

} // namespace cavernbloom

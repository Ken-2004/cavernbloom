#pragma once

#include <glm/vec2.hpp>
#include <cstddef>

namespace cavernbloom::presentation {

// Positive periodic offset. Repeating motifs share this period to avoid wrap seams.
[[nodiscard]] float parallaxOffset(float cameraX, float factor, float period) noexcept;

struct HudLayout {
    glm::vec2 center;
    glm::vec2 size;
    glm::vec2 firstMarker;
    float markerStep;
    float scale;
};

// View dimensions are the camera's positive logical dimensions, not world positions.
[[nodiscard]] HudLayout hudLayout(glm::vec2 viewSize, std::size_t markerCount) noexcept;

} // namespace cavernbloom::presentation

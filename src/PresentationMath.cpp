#include "PresentationMath.hpp"

#include <algorithm>
#include <cmath>

namespace cavernbloom::presentation {

float parallaxOffset(float cameraX, float factor, float period) noexcept
{
    if (!std::isfinite(cameraX) || !std::isfinite(factor) || !std::isfinite(period) || period <= 0.0F) {
        return 0.0F;
    }
    const double offset = std::fmod(static_cast<double>(cameraX) * factor, static_cast<double>(period));
    const float wrapped = static_cast<float>(offset < 0.0 ? offset + period : offset);
    return wrapped < period ? wrapped : 0.0F;
}

HudLayout hudLayout(glm::vec2 viewSize, std::size_t markerCount) noexcept
{
    const float margin = std::min(20.0F, viewSize.x * 0.05F);
    const float naturalWidth = 32.0F + static_cast<float>(markerCount) * 28.0F;
    const float scale = std::min({1.0F, (viewSize.x - margin * 2.0F) / naturalWidth, viewSize.y / 100.0F});
    const glm::vec2 size{naturalWidth * scale, 56.0F * scale};
    const glm::vec2 topLeft{margin, viewSize.y - std::min(margin, viewSize.y * 0.05F)};
    return {topLeft + glm::vec2(size.x * 0.5F, -size.y * 0.5F), size,
            topLeft + glm::vec2(30.0F, -28.0F) * scale, 28.0F * scale, scale};
}

} // namespace cavernbloom::presentation

#pragma once

#include <glm/vec2.hpp>

namespace cavernbloom {

// World-space minimum and maximum corners, shared without a dependency on level generation.
struct Bounds2D {
    glm::vec2 minimum{0.0F};
    glm::vec2 maximum{0.0F};
};

} // namespace cavernbloom

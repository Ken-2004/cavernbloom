#pragma once

#include <glm/vec2.hpp>

namespace cavernbloom {

// Static AABB: position is its center, size is its full positive extent; +Y is up.
struct Platform {
    glm::vec2 position;
    glm::vec2 size;
};

} // namespace cavernbloom

#pragma once

#include <glm/vec2.hpp>

namespace cavernbloom {

// World-space center and full size, with +Y up. No rendering resources.
struct Collectible {
    glm::vec2 position;
    glm::vec2 size;
    bool collected = false;
};

} // namespace cavernbloom

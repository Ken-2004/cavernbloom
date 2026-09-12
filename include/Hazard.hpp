#pragma once

#include <glm/vec2.hpp>
#include <cstddef>

namespace cavernbloom {

// Static trigger, using a world-space center and full size (+Y up).
struct Hazard {
    glm::vec2 position;
    glm::vec2 size;
    std::size_t platformIndex = 0;
};

} // namespace cavernbloom

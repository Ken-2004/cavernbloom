#pragma once

#include <glm/vec2.hpp>
#include <cstddef>

namespace cavernbloom {

struct Enemy {
    glm::vec2 position;
    glm::vec2 size;
    float patrolMinX;
    float patrolMaxX;
    float speed;
    int direction = 1;
    std::size_t platformIndex = 0;
};

} // namespace cavernbloom

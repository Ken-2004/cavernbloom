#include "Collision.hpp"

#include <algorithm>

namespace cavernbloom::collision {
namespace {

bool overlapsInterval(float center, float halfExtent, float minimum, float maximum) noexcept
{
    // Compare centers to the same expanded faces used for resolution so exact
    // contact remains contact even when subtracting half-extents would round.
    return center > minimum - halfExtent && center < maximum + halfExtent;
}

} // namespace

bool overlaps(const Platform& first, const Platform& second) noexcept
{
    const glm::vec2 firstHalf = first.size * 0.5F;
    const glm::vec2 secondHalf = second.size * 0.5F;
    return overlapsInterval(first.position.x, firstHalf.x,
                            second.position.x - secondHalf.x, second.position.x + secondHalf.x)
           && overlapsInterval(first.position.y, firstHalf.y,
                               second.position.y - secondHalf.y, second.position.y + secondHalf.y);
}

MovementResult moveAndResolve(const glm::vec2& position, const glm::vec2& size,
                             const glm::vec2& velocity, float deltaSeconds,
                             std::span<const Platform> platforms) noexcept
{
    MovementResult result{position, velocity};
    const glm::vec2 half = size * 0.5F;
    result.position.x += velocity.x * deltaSeconds;
    bool blockedX = false;

    for (const Platform& platform : platforms) {
        const glm::vec2 platformHalf = platform.size * 0.5F;
        const glm::vec2 minimum = platform.position - platformHalf;
        const glm::vec2 maximum = platform.position + platformHalf;
        if (!overlapsInterval(position.y, half.y, minimum.y, maximum.y)) {
            continue;
        }
        if (velocity.x > 0.0F && position.x <= minimum.x - half.x
            && result.position.x >= minimum.x - half.x) {
            result.position.x = std::min(result.position.x, minimum.x - half.x);
            blockedX = true;
        } else if (velocity.x < 0.0F && position.x >= maximum.x + half.x
                   && result.position.x <= maximum.x + half.x) {
            result.position.x = std::max(result.position.x, maximum.x + half.x);
            blockedX = true;
        }
    }
    if (blockedX) {
        result.velocity.x = 0.0F;
    }

    result.position.y += velocity.y * deltaSeconds;
    bool blockedY = false;
    for (const Platform& platform : platforms) {
        const glm::vec2 platformHalf = platform.size * 0.5F;
        const glm::vec2 minimum = platform.position - platformHalf;
        const glm::vec2 maximum = platform.position + platformHalf;
        if (!overlapsInterval(result.position.x, half.x, minimum.x, maximum.x)) {
            continue;
        }
        if (velocity.y <= 0.0F && position.y >= maximum.y + half.y
            && result.position.y <= maximum.y + half.y) {
            result.position.y = std::max(result.position.y, maximum.y + half.y);
            result.grounded = true;
            blockedY = true;
        } else if (velocity.y > 0.0F && position.y <= minimum.y - half.y
                   && result.position.y >= minimum.y - half.y) {
            result.position.y = std::min(result.position.y, minimum.y - half.y);
            blockedY = true;
        }
    }
    if (blockedY) {
        result.velocity.y = 0.0F;
    }
    return result;
}

} // namespace cavernbloom::collision

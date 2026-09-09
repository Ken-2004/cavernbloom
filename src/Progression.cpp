#include "Progression.hpp"
#include "Collision.hpp"

namespace cavernbloom {

Progression::Progression(std::span<const Collectible> layout, const Platform& goalZone)
    : collectibles_(layout.begin(), layout.end()), goalZone_(goalZone)
{
    reset();
}

void Progression::reset() noexcept
{
    for (Collectible& collectible : collectibles_) {
        collectible.collected = false;
    }
    collectedCount_ = 0;
    state_ = GameState::Playing;
}

bool Progression::update(const Platform& playerBounds) noexcept
{
    if (state_ == GameState::Won) {
        return false;
    }
    for (Collectible& collectible : collectibles_) {
        if (!collectible.collected
            && collision::overlaps(playerBounds, {collectible.position, collectible.size})) {
            collectible.collected = true;
            ++collectedCount_;
        }
    }
    if (allCollected() && collision::overlaps(playerBounds, goalZone_)) {
        state_ = GameState::Won;
        return true;
    }
    return false;
}

} // namespace cavernbloom

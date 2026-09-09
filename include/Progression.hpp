#pragma once

#include "Collectible.hpp"
#include "Platform.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cavernbloom {

enum class GameState { Playing, Won };

class Progression final {
public:
    Progression(std::span<const Collectible> layout, const Platform& goalZone);

    void reset() noexcept;
    // World-space triggers only; returns true on the single transition to Won.
    bool update(const Platform& playerBounds) noexcept;

    [[nodiscard]] std::span<const Collectible> collectibles() const noexcept { return collectibles_; }
    [[nodiscard]] std::size_t collectedCount() const noexcept { return collectedCount_; }
    [[nodiscard]] std::size_t totalCount() const noexcept { return collectibles_.size(); }
    [[nodiscard]] bool allCollected() const noexcept { return collectedCount_ == totalCount(); }
    [[nodiscard]] GameState state() const noexcept { return state_; }

private:
    // Own mutable collection flags; the generated layout remains reusable on restart.
    std::vector<Collectible> collectibles_;
    Platform goalZone_;
    std::size_t collectedCount_ = 0;
    GameState state_ = GameState::Playing;
};

} // namespace cavernbloom

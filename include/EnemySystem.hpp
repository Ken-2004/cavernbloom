#pragma once

#include "Enemy.hpp"
#include <span>
#include <vector>

namespace cavernbloom {

class EnemySystem final {
public:
    explicit EnemySystem(std::span<const Enemy> layout);
    // Exactly one simulation tick; rendering elapsed time never enters patrol movement.
    void update() noexcept;
    void reset() noexcept;
    [[nodiscard]] std::span<const Enemy> enemies() const noexcept { return enemies_; }

private:
    std::vector<Enemy> initial_;
    std::vector<Enemy> enemies_;
};

} // namespace cavernbloom

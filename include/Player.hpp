#pragma once

#include "SimulationConfig.hpp"
#include "Platform.hpp"

#include <span>

namespace cavernbloom {

class Player final {
public:
    explicit Player(glm::vec2 spawnPosition = simulation::playerSpawn) noexcept;
    // jumpPressed is a one-step request, not the held state of the key.
    // Returns whether this step accepted a grounded jump.
    bool update(float deltaSeconds, int horizontalDirection, bool jumpPressed,
                std::span<const Platform> platforms) noexcept;
    void resetToSpawn() noexcept;

    [[nodiscard]] const glm::vec2& position() const noexcept { return position_; }
    [[nodiscard]] const glm::vec2& size() const noexcept { return size_; }
    [[nodiscard]] const glm::vec2& velocity() const noexcept { return velocity_; }
    [[nodiscard]] bool grounded() const noexcept { return grounded_; }

private:
    glm::vec2 spawnPosition_;
    glm::vec2 position_;
    glm::vec2 velocity_{0.0F};
    glm::vec2 size_{simulation::playerSize};
    bool grounded_ = false;
};

} // namespace cavernbloom

#pragma once

#include "SimulationConfig.hpp"

namespace cavernbloom {

class Player final {
public:
    // jumpPressed is a one-step request, not the held state of the key.
    void update(float deltaSeconds, int horizontalDirection, bool jumpPressed) noexcept;

    [[nodiscard]] const glm::vec2& position() const noexcept { return position_; }
    [[nodiscard]] const glm::vec2& size() const noexcept { return size_; }

private:
    glm::vec2 position_{simulation::playerSpawn};
    glm::vec2 velocity_{0.0F};
    glm::vec2 size_{simulation::playerSize};
    bool grounded_ = false;
};

} // namespace cavernbloom

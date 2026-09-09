#pragma once

#include "Platform.hpp"
#include "SimulationConfig.hpp"

#include <optional>

namespace cavernbloom {

struct JumpParameters {
    glm::vec2 playerSize = simulation::playerSize;
    double moveSpeed = simulation::moveSpeed;
    double jumpSpeed = simulation::jumpSpeed;
    double gravity = simulation::gravity;
    double safetyFactor = simulation::reachabilityFactor;
};

// Top-to-top ballistic feasibility with freely chosen takeoff/landing positions.
// Assumes constant horizontal speed, optional horizontal stopping, and no obstacles
// along the arc. The generator must provide clear geometry. Not a path finder.
class JumpReachability final {
public:
    explicit JumpReachability(JumpParameters parameters = {});
    [[nodiscard]] double maximumJumpHeight() const noexcept;
    [[nodiscard]] std::optional<double> descendingFlightTime(double verticalDisplacement) const noexcept;
    [[nodiscard]] std::optional<double> maximumHorizontalTravel(double verticalDisplacement) const noexcept;
    [[nodiscard]] bool canReach(const Platform& source, const Platform& target) const noexcept;

private:
    JumpParameters parameters_;
};

} // namespace cavernbloom

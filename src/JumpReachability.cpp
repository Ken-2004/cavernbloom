#include "JumpReachability.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cavernbloom {
namespace {

bool validPlatform(const Platform& platform) noexcept
{
    return std::isfinite(platform.position.x) && std::isfinite(platform.position.y)
           && std::isfinite(platform.size.x) && std::isfinite(platform.size.y)
           && platform.size.x > 0.0F && platform.size.y > 0.0F;
}

} // namespace

JumpReachability::JumpReachability(JumpParameters parameters) : parameters_(parameters)
{
    if (!std::isfinite(parameters.playerSize.x) || !std::isfinite(parameters.playerSize.y)
        || parameters.playerSize.x <= 0.0F || parameters.playerSize.y <= 0.0F
        || !std::isfinite(parameters.moveSpeed) || parameters.moveSpeed <= 0.0
        || !std::isfinite(parameters.jumpSpeed) || parameters.jumpSpeed <= 0.0
        || !std::isfinite(parameters.gravity) || parameters.gravity >= 0.0
        || !std::isfinite(parameters.safetyFactor) || parameters.safetyFactor <= 0.0
        || parameters.safetyFactor > 1.0 || !std::isfinite(maximumJumpHeight())
        || !maximumHorizontalTravel(0.0).has_value()) {
        throw std::invalid_argument("Jump parameters must be finite and physical; safety factor must be in (0, 1].");
    }
}

double JumpReachability::maximumJumpHeight() const noexcept
{
    return -parameters_.jumpSpeed * parameters_.jumpSpeed / (2.0 * parameters_.gravity);
}

std::optional<double> JumpReachability::descendingFlightTime(double verticalDisplacement) const noexcept
{
    if (!std::isfinite(verticalDisplacement) || verticalDisplacement > maximumJumpHeight()) {
        return std::nullopt;
    }
    // The minus-square-root branch has downward velocity when gravity is negative.
    const double discriminant = parameters_.jumpSpeed * parameters_.jumpSpeed
                                + 2.0 * parameters_.gravity * verticalDisplacement;
    if (!std::isfinite(discriminant) || discriminant < 0.0) {
        return std::nullopt;
    }
    const double time = (-parameters_.jumpSpeed - std::sqrt(discriminant)) / parameters_.gravity;
    return std::isfinite(time) && time > 0.0 ? std::optional<double>(time) : std::nullopt;
}

std::optional<double> JumpReachability::maximumHorizontalTravel(double verticalDisplacement) const noexcept
{
    const auto time = descendingFlightTime(verticalDisplacement);
    if (!time) {
        return std::nullopt;
    }
    const double distance = parameters_.moveSpeed * *time;
    return std::isfinite(distance) ? std::optional<double>(distance) : std::nullopt;
}

bool JumpReachability::canReach(const Platform& source, const Platform& target) const noexcept
{
    if (!validPlatform(source) || !validPlatform(target)) {
        return false;
    }
    const double sourceTop = static_cast<double>(source.position.y) + source.size.y * 0.5;
    const double targetTop = static_cast<double>(target.position.y) + target.size.y * 0.5;
    const double displacement = targetTop - sourceTop;
    if (displacement > maximumJumpHeight() * parameters_.safetyFactor) {
        return false;
    }
    const auto travel = maximumHorizontalTravel(displacement);
    if (!travel) {
        return false;
    }

    // Require this much player-width support at BOTH ends. This still allows a
    // modest overhang instead of demanding that platform centers be reachable.
    const double requiredOverlap = parameters_.playerSize.x * parameters_.safetyFactor;
    if (source.size.x < requiredOverlap || target.size.x < requiredOverlap) {
        return false;
    }
    const double sourceBand = (source.size.x + static_cast<double>(parameters_.playerSize.x)) * 0.5
                              - requiredOverlap;
    const double targetBand = (target.size.x + static_cast<double>(parameters_.playerSize.x)) * 0.5
                              - requiredOverlap;
    const double minimumTravel = std::max(0.0,
        std::abs(static_cast<double>(target.position.x) - source.position.x) - sourceBand - targetBand);
    return minimumTravel <= *travel * parameters_.safetyFactor;
}

} // namespace cavernbloom

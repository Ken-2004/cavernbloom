#include "JumpReachability.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

using cavernbloom::JumpParameters;
using cavernbloom::JumpReachability;
using cavernbloom::Platform;

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(double first, double second)
{
    return std::abs(first - second) < 1.0e-8;
}

Platform platform(float x, float top, float width = 64.0F)
{
    return {{x, top - 10.0F}, {width, 20.0F}};
}

void checkPhysics()
{
    const JumpParameters parameters;
    const JumpReachability model;
    const double apex = parameters.jumpSpeed * parameters.jumpSpeed / (-2.0 * parameters.gravity);
    const double sameTime = -2.0 * parameters.jumpSpeed / parameters.gravity;
    require(near(model.maximumJumpHeight(), apex), "Apex formula mismatch.");
    require(near(*model.descendingFlightTime(0.0), sameTime), "Same-height time mismatch.");
    require(near(*model.maximumHorizontalTravel(0.0), parameters.moveSpeed * sameTime),
            "Horizontal travel formula mismatch.");
    for (const double height : {0.0, apex * 0.5, -apex}) {
        const auto time = model.descendingFlightTime(height);
        require(time.has_value(), "Valid height has no flight time.");
        require(near(parameters.jumpSpeed * *time + 0.5 * parameters.gravity * *time * *time, height),
                "Landing time does not solve ballistic equation.");
        require(parameters.jumpSpeed + parameters.gravity * *time < 0.0, "Selected ascending root.");
    }
    require(model.descendingFlightTime(apex).has_value(), "Apex time should be defined.");
    require(!model.descendingFlightTime(apex + 1.0), "Above-apex target has a flight time.");
    require(*model.descendingFlightTime(-apex) > sameTime, "Lower target did not extend flight.");
    std::cout << "Apex=" << apex << "; same-height time=" << sameTime
              << "; travel=" << *model.maximumHorizontalTravel(0.0)
              << "; safe travel=" << *model.maximumHorizontalTravel(0.0) * parameters.safetyFactor << '\n';
}

void checkReachability()
{
    const JumpParameters parameters;
    const JumpReachability model;
    const Platform source = platform(0.0F, 0.0F);
    // Safe source/target center bands extend 20 units from these 64-unit platforms.
    const double band = (source.size.x + parameters.playerSize.x) * 0.5
                        - parameters.playerSize.x * parameters.safetyFactor;
    const double safeTravel = *model.maximumHorizontalTravel(0.0) * parameters.safetyFactor;
    const float inside = static_cast<float>(safeTravel + 2.0 * band - 2.0);
    const float outside = static_cast<float>(safeTravel + 2.0 * band + 2.0);
    require(model.canReach(source, platform(inside, 0.0F)), "Safe same-height jump rejected.");
    require(!model.canReach(source, platform(outside, 0.0F)), "Out-of-range jump accepted.");
    require(model.canReach(source, platform(100.0F, 40.0F)), "Moderate upward jump rejected.");
    require(!model.canReach(source, platform(50.0F, static_cast<float>(model.maximumJumpHeight() + 1.0))),
            "Above-apex jump accepted.");
    require(!model.canReach(source, platform(50.0F, static_cast<float>(model.maximumJumpHeight() * 0.9))),
            "Vertical safety reserve ignored.");
    require(model.canReach(source, platform(outside, -80.0F)), "Lower target did not allow a longer jump.");
    require(model.canReach(source, platform(-inside, 0.0F)), "Left/right geometry is asymmetric.");
    require(!model.canReach(source, platform(-outside, 0.0F)), "Leftward range limit ignored.");

    JumpParameters exact = parameters;
    exact.safetyFactor = 1.0;
    const JumpReachability unreserved(exact);
    const float borderline = static_cast<float>(*model.maximumHorizontalTravel(0.0) * 0.95
                                                + source.size.x - parameters.playerSize.x);
    require(unreserved.canReach(source, platform(borderline, 0.0F))
                && !model.canReach(source, platform(borderline, 0.0F)), "Safety factor did not reject a borderline jump.");
    require(model.canReach(source, platform(outside, 0.0F, 96.0F)), "Target width was not accounted for.");
    require(model.canReach(platform(0.0F, 0.0F, 96.0F), platform(outside, 0.0F)),
            "Source width was not accounted for.");
    require(!model.canReach(source, platform(50.0F, 0.0F, 31.0F)), "Insufficient landing support accepted.");
    JumpParameters smaller = parameters;
    smaller.playerSize.x = 20.0F;
    require(JumpReachability(smaller).canReach(source, platform(50.0F, 0.0F, 31.0F)),
            "Player width did not affect required overlap.");
    JumpParameters taller = parameters;
    taller.playerSize.y *= 2.0F;
    require(JumpReachability(taller).canReach(source, platform(inside, 0.0F)),
            "Equal takeoff/landing half-height offsets should cancel.");
    for (int repeat = 0; repeat < 100; ++repeat) {
        require(model.canReach(source, platform(inside, 0.0F)), "Repeated query changed result.");
    }
}

void checkInvalidInputs()
{
    const JumpReachability model;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    require(!model.descendingFlightTime(nan) && !model.maximumHorizontalTravel(nan), "NaN accepted.");
    require(!model.descendingFlightTime(-std::numeric_limits<double>::infinity()), "Infinity accepted.");
    require(!model.canReach(platform(0.0F, 0.0F), platform(10.0F, 0.0F, 0.0F)), "Zero size accepted.");
    require(!model.canReach(platform(0.0F, 0.0F), platform(std::numeric_limits<float>::infinity(), 0.0F)),
            "Infinite platform accepted.");
    for (int field = 0; field < 7; ++field) {
        JumpParameters invalid;
        switch (field) {
        case 0: invalid.gravity = 0.0; break;
        case 1: invalid.gravity = 1.0; break;
        case 2: invalid.jumpSpeed = -1.0; break;
        case 3: invalid.moveSpeed = nan; break;
        case 4: invalid.safetyFactor = 0.0; break;
        case 5: invalid.safetyFactor = 1.1; break;
        default: invalid.playerSize.x = -1.0F; break;
        }
        bool rejected = false;
        try { const JumpReachability invalidModel(invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Nonphysical parameters were not rejected.");
    }
}

} // namespace

int main()
{
    try {
        checkPhysics();
        checkReachability();
        checkInvalidInputs();
        std::cout << "Ballistic reachability checks passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

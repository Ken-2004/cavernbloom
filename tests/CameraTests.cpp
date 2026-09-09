#include "Camera2D.hpp"
#include "Player.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

using cavernbloom::Camera2D;
using cavernbloom::Bounds2D;
constexpr Bounds2D world{{-600.0F, -280.0F}, {2400.0F, -100.0F}};

void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}

bool near(float first, float second)
{
    return std::abs(first - second) < 0.0001F;
}

void requireFinite(const Camera2D& camera)
{
    require(std::isfinite(camera.center().x) && std::isfinite(camera.center().y), "Nonfinite center.");
    const glm::mat4 matrix = camera.viewProjection();
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            require(std::isfinite(matrix[column][row]), "Nonfinite view-projection matrix.");
        }
    }
}

void checkFollowAndClamping()
{
    Camera2D camera;
    require(camera.center() == glm::vec2(0.0F), "Invalid initial center.");
    requireFinite(camera);
    camera.setWorldBounds(world);
    camera.recenter({1000.0F, 80.0F});
    require(camera.center() == glm::vec2(1000.0F, 0.0F), "Recenter altered fixed Y.");
    camera.follow({1100.0F, 300.0F});
    require(camera.center().x == 1000.0F, "Inside dead zone moved camera.");
    camera.follow({1120.0F, -200.0F});
    require(camera.center().x == 1000.0F, "Dead-zone boundary moved camera.");
    camera.follow({1200.0F, 0.0F});
    require(camera.center().x == 1080.0F, "Right follow failed.");
    camera.follow({800.0F, 0.0F});
    require(camera.center().x == 920.0F, "Left follow failed.");
    for (int update = 0; update < 1000; ++update) {
        camera.follow({800.0F, static_cast<float>(update)});
        require(camera.center() == glm::vec2(920.0F, 0.0F), "Stationary X caused camera jitter.");
    }
    camera.follow({-2000.0F, 0.0F});
    require(camera.center().x - camera.visibleSize().x * 0.5F == world.minimum.x, "Start clamp failed.");
    camera.follow({5000.0F, 0.0F});
    require(camera.center().x + camera.visibleSize().x * 0.5F == world.maximum.x, "End clamp failed.");
    camera.recenter({-528.0F, -184.0F});
    require(camera.center().x == 40.0F, "Spawn recenter did not apply start clamp.");
    require(std::abs(-528.0F - camera.center().x) < camera.visibleSize().x * 0.5F,
            "Spawn is invisible after recenter.");
}

void checkResizeAndMatrices()
{
    Camera2D camera;
    camera.setWorldBounds(world);
    const glm::vec2 target{1000.0F, -208.0F};
    camera.recenter(target);
    const glm::vec2 originalTarget = target;
    camera.setViewportSize(800, 800);
    require(camera.visibleSize() == glm::vec2(720.0F), "Square resize did not preserve aspect ratio.");
    require(target == originalTarget && camera.center().x == target.x, "Resize altered target or unclamped center.");
    const glm::mat4 matrix = camera.viewProjection();
    const glm::vec4 origin = matrix * glm::vec4(camera.center(), 0.0F, 1.0F);
    const glm::vec4 upperRight = matrix * glm::vec4(camera.center() + camera.visibleSize() * 0.5F, 0.0F, 1.0F);
    require(near(origin.x, 0.0F) && near(origin.y, 0.0F), "Camera center does not map to view center.");
    require(near(upperRight.x, 1.0F) && near(upperRight.y, 1.0F), "View/projection scale or order is incorrect.");
    camera.setViewportSize(1920, 1080);
    require(near(camera.visibleSize().x, 1280.0F), "16:9 resize changed world scale.");
    camera.recenter({world.maximum.x, 0.0F});
    camera.setViewportSize(1600, 720);
    require(camera.center().x + camera.visibleSize().x * 0.5F == world.maximum.x, "Resize did not reclamp at end.");
    camera.setViewportSize(4000, 720);
    require(camera.center().x == 900.0F, "Viewport wider than world should center the world.");
    camera.follow({world.maximum.x, 0.0F});
    require(camera.center().x == 900.0F, "Oversized viewport follow broke balanced margins.");
    camera.setViewportSize(100, 720);
    camera.recenter({1000.0F, 0.0F});
    camera.follow({1040.0F, 0.0F});
    require(camera.center().x == 1015.0F, "Narrow-view dead zone was not reduced.");
}

void checkInvalidInputsAndDeterminism()
{
    Camera2D first;
    first.setWorldBounds(world);
    const glm::vec2 size = first.visibleSize();
    first.setViewportSize(0, 0);
    first.setViewportSize(-1, 720);
    first.setViewportSize(1280, 0);
    require(first.visibleSize() == size, "Invalid/minimized size corrupted view.");
    const glm::vec2 center = first.center();
    first.follow({std::numeric_limits<float>::infinity(), 0.0F});
    first.recenter({0.0F, std::numeric_limits<float>::quiet_NaN()});
    require(first.center() == center, "Invalid target corrupted center.");
    bool rejected = false;
    try { first.setWorldBounds({{1.0F, 0.0F}, {-1.0F, 0.0F}}); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected && first.center() == center, "Inverted bounds were not safely rejected.");
    Camera2D second = first;
    for (int frame = 0; frame < 2000; ++frame) {
        const glm::vec2 target{static_cast<float>(frame % 1700), static_cast<float>(frame % 300)};
        if (frame % 13 == 0) {
            first.setViewportSize(800 + frame % 600, 720);
            second.setViewportSize(800 + frame % 600, 720);
        }
        first.follow(target);
        second.follow(target);
        require(first.center() == second.center() && first.visibleSize() == second.visibleSize(),
                "Identical camera updates diverged.");
        requireFinite(first);
    }
    first.setViewportSize(std::numeric_limits<int>::max(), 1);
    requireFinite(first);
}

void checkPhysicsIndependence()
{
    constexpr std::array<cavernbloom::Platform, 1> platforms{{{{0.0F, -260.0F}, {4000.0F, 40.0F}}}};
    const auto originalPlatforms = platforms;
    cavernbloom::Player observed;
    cavernbloom::Player baseline;
    Camera2D camera;
    camera.setWorldBounds(world);
    for (int tick = 0; tick < 2000; ++tick) {
        const int direction = (tick / 120) % 2 == 0 ? 1 : -1;
        const bool jump = tick % 101 == 0;
        const float step = static_cast<float>(cavernbloom::simulation::fixedStepSeconds);
        observed.update(step, direction, jump, platforms);
        baseline.update(step, direction, jump, platforms);
        camera.setViewportSize(600 + tick % 1000, 720);
        camera.follow(observed.position());
        if (tick % 97 == 0) { camera.recenter(observed.position()); }
        require(observed.position() == baseline.position() && observed.velocity() == baseline.velocity()
                    && observed.grounded() == baseline.grounded(), "Camera changed physics state.");
        require(platforms[0].position == originalPlatforms[0].position && platforms[0].size == originalPlatforms[0].size,
                "Camera changed platform geometry.");
    }
}

} // namespace

int main()
{
    try {
        checkFollowAndClamping();
        checkResizeAndMatrices();
        checkInvalidInputsAndDeterminism();
        checkPhysicsIndependence();
        std::cout << "Camera follow, clamps, resize, matrices, reset, finiteness and physics independence passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

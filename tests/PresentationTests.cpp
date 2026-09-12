#include "PresentationMath.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <stdexcept>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}
bool near(float a, float b) { return std::abs(a - b) < 0.001F; }
} // namespace

int main()
{
    using namespace cavernbloom::presentation;
    try {
        require(near(parallaxOffset(100.0F, 0.25F, 1000.0F), 25.0F), "Wrong parallax rate.");
        require(near(parallaxOffset(-100.0F, 0.25F, 1000.0F), 975.0F), "Negative camera wrap failed.");
        require(near(parallaxOffset(4000.0F, 0.25F, 1000.0F), 0.0F), "Exact period did not wrap.");
        for (int tick = -10000; tick < 10000; ++tick) {
            const float x = static_cast<float>(tick);
            const float first = parallaxOffset(x, 0.25F, 1000.0F);
            require(first >= 0.0F && first < 1000.0F && std::isfinite(first), "Offset escaped its period.");
            require(first == parallaxOffset(x, 0.25F, 1000.0F)
                        && near(first, parallaxOffset(x + 4000.0F, 0.25F, 1000.0F)), "Repeated motifs are not deterministic/periodic.");
            const float next = parallaxOffset(x + 1.0F, 0.25F, 1000.0F);
            const float movement = next < first ? next + 1000.0F - first : next - first;
            require(near(movement, 0.25F), "Parallax wrap introduced a discontinuity.");
        }
        require(parallaxOffset(1.0F, 0.25F, 0.0F) == 0.0F
                    && parallaxOffset(std::numeric_limits<float>::infinity(), 0.25F, 1000.0F) == 0.0F,
                "Invalid parallax input did not fall back safely.");
        constexpr std::array<glm::vec2, 6> views{{{1280.0F, 720.0F}, {720.0F, 720.0F}, {200.0F, 720.0F},
                                                {40.0F, 720.0F}, {4000.0F, 720.0F}, {1280.0F, 40.0F}}};
        for (const glm::vec2 view : views) {
            for (const std::size_t count : {std::size_t{0}, std::size_t{8}, std::size_t{10}}) {
                const auto layout = hudLayout(view, count);
                const glm::vec2 minimum = layout.center - layout.size * 0.5F;
                const glm::vec2 maximum = layout.center + layout.size * 0.5F;
                require(std::isfinite(layout.scale) && layout.scale > 0.0F && layout.scale <= 1.0F
                            && minimum.x >= 0.0F && minimum.y >= 0.0F
                            && maximum.x <= view.x && maximum.y <= view.y, "HUD panel escaped viewport.");
                for (std::size_t index = 0; index < count; ++index) {
                    const glm::vec2 marker = layout.firstMarker + glm::vec2(static_cast<float>(index) * layout.markerStep, 0.0F);
                    const glm::vec2 halfSize = glm::vec2(7.0F, 11.0F) * layout.scale;
                    require(marker.x - halfSize.x >= minimum.x && marker.x + halfSize.x <= maximum.x
                                && marker.y - halfSize.y >= minimum.y && marker.y + halfSize.y <= maximum.y,
                            "HUD marker clipped its panel.");
                }
                require(layout.center == hudLayout(view, count).center, "Repeated HUD layout changed.");
            }
        }
        const auto wide = hudLayout({1280.0F, 720.0F}, 8);
        const auto square = hudLayout({720.0F, 720.0F}, 8);
        require(wide.firstMarker == square.firstMarker && wide.markerStep == square.markerStep,
                "Ordinary aspect changes moved the top-left HUD anchor.");
        std::cout << "Parallax rate/wrapping/determinism and HUD bounds at wide, square, narrow and short sizes passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

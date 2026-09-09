#include "Progression.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}
} // namespace

int main()
{
    using namespace cavernbloom;
    try {
        const std::array<Collectible, 2> layout{{{{0.0F, 0.0F}, {14.0F, 18.0F}},
                                               {{100.0F, 0.0F}, {14.0F, 18.0F}}}};
        Progression progress(layout, {{500.0F, 0.0F}, {20.0F, 72.0F}});
        require(progress.totalCount() == 2 && progress.collectedCount() == 0
                    && !progress.allCollected(), "Invalid initial counts.");
        progress.update({{50.0F, 0.0F}, {40.0F, 64.0F}});
        progress.update({{27.0F, 0.0F}, {40.0F, 64.0F}});
        progress.update({{0.0F, 41.0F}, {40.0F, 64.0F}});
        require(progress.collectedCount() == 0, "Separated or edge-touching bounds collected a flower.");
        progress.update({{26.0F, 0.0F}, {40.0F, 64.0F}});
        require(progress.collectedCount() == 1 && progress.collectibles()[0].collected
                    && !progress.allCollected(), "Positive overlap did not collect exactly one flower.");
        for (int tick = 0; tick < 1000; ++tick) {
            progress.update({{0.0F, 0.0F}, {40.0F, 64.0F}});
            require(progress.collectedCount() == 1 && progress.collectedCount() <= progress.totalCount(),
                    "Repeated overlap changed count.");
        }
        progress.update({{100.0F, 0.0F}, {40.0F, 64.0F}});
        require(progress.allCollected() && progress.collectedCount() == 2, "Last flower did not unlock progress.");
        require(!layout[0].collected && !layout[1].collected, "Progression mutated the source layout.");
        for (const Collectible& flower : progress.collectibles()) {
            require(std::isfinite(flower.position.x) && std::isfinite(flower.position.y)
                        && std::isfinite(flower.size.x) && std::isfinite(flower.size.y)
                        && flower.size.x > 0.0F && flower.size.y > 0.0F, "Invalid flower geometry.");
        }
        progress.reset();
        require(progress.collectedCount() == 0 && progress.totalCount() == 2 && !progress.allCollected(),
                "Reset corrupted counts.");
        for (const Collectible& flower : progress.collectibles()) {
            require(!flower.collected, "Reset did not restore a flower.");
        }
        std::cout << "Collectible overlap, separation/contact, exact-once counts, geometry and restoration passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

#include "Progression.hpp"
#include "LevelGenerator.hpp"
#include "SimulationConfig.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
using namespace cavernbloom;

void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}

void checkGeneratedFlow()
{
    const auto level = generateLevel(generation::developmentSeed);
    Progression first(level.collectibles, level.goalZone);
    Progression repeated(level.collectibles, level.goalZone);
    for (int run = 0; run < 2; ++run) {
        require(!first.update(level.goalZone) && first.state() == GameState::Playing,
                "Locked goal completed the level.");
        require(!repeated.update(level.goalZone), "Repeated locked goal differs.");
        for (std::size_t index = 0; index < level.collectibles.size(); ++index) {
            const Platform player{level.collectibles[index].position, simulation::playerSize};
            require(!first.update(player) && !repeated.update(player), "Flower alone won the level.");
            require(first.collectedCount() == index + 1 && first.collectedCount() == repeated.collectedCount(),
                    "Deterministic collection flow failed.");
            require(first.allCollected() == (index + 1 == level.collectibles.size()), "Goal unlocked early.");
        }
        const Platform touching{{level.goalZone.position.x + level.goalZone.size.x, level.goalZone.position.y},
                                level.goalZone.size};
        require(!first.update(touching), "Goal edge contact triggered win.");
        require(first.state() == GameState::Playing, "All flowers won without reaching the goal.");
        require(first.update(level.goalZone) && repeated.update(level.goalZone), "Unlocked goal did not win.");
        for (int tick = 0; tick < 1000; ++tick) {
            require(!first.update(level.goalZone) && !repeated.update(level.goalZone), "Win fired more than once.");
            require(first.state() == GameState::Won && first.state() == repeated.state()
                        && first.collectedCount() == first.totalCount(), "Won state or counts changed.");
        }
        first.reset();
        repeated.reset();
        require(first.state() == GameState::Playing && first.collectedCount() == 0
                    && first.totalCount() == level.collectibles.size(), "Restart did not restore Playing/counts.");
        for (std::size_t index = 0; index < level.collectibles.size(); ++index) {
            const auto& flower = first.collectibles()[index];
            require(!flower.collected && flower.position == level.collectibles[index].position
                        && flower.size == level.collectibles[index].size, "Restart changed flower layout.");
        }
    }
}
} // namespace

int main()
{
    try {
        checkGeneratedFlow();
        const Platform goal{{100.0F, 0.0F}, {20.0F, 72.0F}};
        Progression empty({}, goal);
        require(empty.allCollected() && empty.totalCount() == 0 && empty.state() == GameState::Playing,
                "Zero-item level should start unlocked, but Playing.");
        require(!empty.update({{0.0F, 0.0F}, {20.0F, 20.0F}}), "Empty level won outside goal.");
        require(empty.update(goal) && !empty.update(goal), "Empty level goal should complete once.");
        empty.reset();
        require(empty.state() == GameState::Playing && empty.collectedCount() == 0, "Empty reset failed.");
        const std::array<Collectible, 1> atGoal{{{goal.position, {10.0F, 10.0F}, true}}};
        Progression simultaneous(atGoal, goal);
        require(simultaneous.collectedCount() == 0 && !simultaneous.collectibles()[0].collected,
                "Constructor trusted stale input flags.");
        require(simultaneous.update(goal) && simultaneous.collectedCount() == 1,
                "Last collection and goal in one step should win.");
        std::cout << "Locked/unlocked goal, single win, deterministic generated flow, restart and zero-item case passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

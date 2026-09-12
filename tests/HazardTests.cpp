#include "Gameplay.hpp"
#include "EncounterConfig.hpp"

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

GeneratedLevel fixture(bool enemy)
{
    GeneratedLevel level;
    level.platforms.push_back({{0.0F, -10.0F}, {800.0F, 20.0F}});
    level.spawnPosition = {-120.0F, simulation::playerSize.y * 0.5F};
    level.collectibles.push_back({{-120.0F, 21.0F}, {14.0F, 18.0F}});
    level.goalZone = {{300.0F, 40.0F}, {20.0F, 72.0F}};
    if (enemy) {
        level.enemies.push_back({{0.0F, 12.0F}, encounters::enemySize, -8.0F, 8.0F, encounters::enemySpeed, -1});
    } else {
        level.hazards.push_back({{0.0F, 6.0F}, encounters::hazardSize});
    }
    return level;
}

void checkFailure(bool enemy)
{
    const auto level = fixture(enemy);
    Gameplay game(level);
    Gameplay duplicate(level);
    require(game.update(0, false) == GameplayEvent::None && duplicate.update(0, false) == GameplayEvent::None,
            "Safe spawn triggered contact.");
    require(game.progression().collectedCount() == 1 && game.progression().allCollected(), "Fixture flower did not collect.");
    int failures = 0;
    for (int tick = 0; tick < 120; ++tick) {
        const auto event = game.update(1, false);
        require(event == duplicate.update(1, false) && game.player().position() == duplicate.player().position(),
                "Contact/reset was not deterministic.");
        if (event != GameplayEvent::None) {
            require(event == (enemy ? GameplayEvent::EnemyContact : GameplayEvent::HazardContact), "Wrong failure cause.");
            ++failures;
            break;
        }
    }
    require(failures == 1 && game.player().position() == level.spawnPosition
                && game.player().velocity() == glm::vec2(0.0F) && !game.player().grounded(), "Contact did not reset player once.");
    require(game.progression().collectedCount() == 1 && game.progression().allCollected()
                && game.progression().state() == GameState::Playing, "Failure lost unlocked progress.");
    if (enemy) {
        require(game.enemies()[0].position == level.enemies[0].position
                    && game.enemies()[0].direction == level.enemies[0].direction, "Failure did not restore patrol.");
    }
    for (int tick = 0; tick < 1000; ++tick) {
        require(game.update(0, false) == GameplayEvent::None, "One contact produced repeated spawn resets.");
    }
    game.restart();
    require(game.player().position() == level.spawnPosition && game.progression().collectedCount() == 0
                && !game.progression().collectibles()[0].collected && game.progression().state() == GameState::Playing,
            "Full restart failed to restore player/flowers/Playing.");
    if (enemy) { require(game.enemies()[0].position == level.enemies[0].position, "R did not restore patrol."); }
}

void checkWonAndFall()
{
    auto level = fixture(true);
    level.goalZone.position.x = level.spawnPosition.x;
    level.hazards.push_back({{60.0F, 6.0F}, encounters::hazardSize});
    Gameplay game(level);
    require(game.update(0, false) == GameplayEvent::Won, "Unlocked overlap did not win.");
    const glm::vec2 player = game.player().position();
    const Enemy patrol = game.enemies()[0];
    for (int tick = 0; tick < 1000; ++tick) {
        require(game.update(1, true) == GameplayEvent::None && game.player().position() == player
                    && game.enemies()[0].position == patrol.position && game.enemies()[0].direction == patrol.direction
                    && game.progression().state() == GameState::Won, "Won allowed simulation or repeated transitions.");
    }
    game.restart();
    require(game.progression().state() == GameState::Playing && game.progression().collectedCount() == 0
                && game.enemies()[0].position == level.enemies[0].position, "Restart from Won failed.");
    GeneratedLevel empty;
    empty.spawnPosition = simulation::playerSpawn;
    empty.collectibles.push_back({empty.spawnPosition, {14.0F, 18.0F}});
    empty.goalZone = {{300.0F, 0.0F}, {20.0F, 72.0F}};
    Gameplay falling(empty);
    bool fell = false;
    for (int tick = 0; tick < 200; ++tick) {
        if (falling.update(0, false) == GameplayEvent::Fell) { fell = true; break; }
    }
    require(fell && falling.player().position() == empty.spawnPosition
                && falling.player().velocity() == glm::vec2(0.0F) && falling.progression().collectedCount() == 1,
            "Falling no longer resets while preserving progress.");
}
} // namespace

int main()
{
    try {
        auto enemyLevel = fixture(true);
        auto hazardLevel = fixture(false);
        require(detectContact({{0.0F, 32.0F}, simulation::playerSize}, enemyLevel.enemies, {}) == GameplayEvent::EnemyContact,
                "Enemy overlap missed.");
        require(detectContact({{0.0F, 32.0F}, simulation::playerSize}, {}, hazardLevel.hazards) == GameplayEvent::HazardContact,
                "Hazard overlap missed.");
        require(detectContact({{30.0F, 32.0F}, simulation::playerSize}, enemyLevel.enemies, {}) == GameplayEvent::None,
                "Enemy edge contact counted as overlap.");
        require(detectContact({{28.0F, 32.0F}, simulation::playerSize}, {}, hazardLevel.hazards) == GameplayEvent::None,
                "Hazard edge contact counted as overlap.");
        require(detectContact({{0.0F, 100.0F}, simulation::playerSize}, enemyLevel.enemies, hazardLevel.hazards)
                    == GameplayEvent::None, "Separated bounds caused failure.");
        const glm::vec2 translation{2000.0F, 300.0F};
        enemyLevel.enemies[0].position += translation;
        hazardLevel.hazards[0].position += translation;
        require(detectContact({glm::vec2(0.0F, 32.0F) + translation, simulation::playerSize}, enemyLevel.enemies, {})
                    == GameplayEvent::EnemyContact
                    && detectContact({glm::vec2(0.0F, 32.0F) + translation, simulation::playerSize}, {}, hazardLevel.hazards)
                    == GameplayEvent::HazardContact, "World translation changed contact math.");
        checkFailure(true);
        checkFailure(false);
        checkWonAndFall();
        std::cout << "Enemy/thorn contact, exact-once safe respawn, preserved flowers/unlock, full restart, fall and Won freeze passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

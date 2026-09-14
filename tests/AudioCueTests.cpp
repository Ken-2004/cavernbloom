#include "AudioCues.hpp"
#include "Gameplay.hpp"
#include "EncounterConfig.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
using namespace cavernbloom;

void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}

void expect(const Gameplay& game, std::size_t jumps, std::size_t collects,
            std::size_t damage, std::size_t wins)
{
    const auto cues = audioCues(game.transitions());
    require(cues[0].effect == SoundEffect::Jump && cues[0].count == jumps, "Wrong jump cue count.");
    require(cues[1].effect == SoundEffect::Collect && cues[1].count == collects, "Wrong collect cue count.");
    require(cues[2].effect == SoundEffect::Damage && cues[2].count == damage, "Wrong damage cue count.");
    require(cues[3].effect == SoundEffect::Win && cues[3].count == wins, "Wrong win cue count.");
}

GeneratedLevel fixture()
{
    GeneratedLevel level;
    level.platforms.push_back({{0.0F, -10.0F}, {800.0F, 20.0F}});
    level.spawnPosition = {-120.0F, simulation::playerSize.y * 0.5F};
    level.goalZone = {{300.0F, 40.0F}, {20.0F, 72.0F}};
    return level;
}

void jumps()
{
    const auto level = fixture();
    Gameplay game(level);
    (void)game.update(0, true); // Spawn is not grounded until the first collision step.
    expect(game, 0, 0, 0, 0);
    require(game.player().grounded(), "Fixture did not land.");
    (void)game.update(0, true);
    expect(game, 1, 0, 0, 0);
    require(game.player().velocity().y > 0.0F, "Jump cue without an actual jump.");
    (void)game.update(0, true); // Another press in midair is rejected.
    expect(game, 0, 0, 0, 0);
    for (int tick = 0; tick < 200; ++tick) {
        (void)game.update(0, false); // A held key produces no new press requests.
        expect(game, 0, 0, 0, 0);
    }
    require(game.player().grounded(), "Player did not land after one jump.");
    (void)game.update(0, true);
    expect(game, 1, 0, 0, 0);
    game.restart();
    expect(game, 0, 0, 0, 0);
}

void pickupsAndWin()
{
    auto level = fixture();
    level.collectibles.push_back({level.spawnPosition, {14.0F, 18.0F}});
    level.collectibles.push_back({level.spawnPosition + glm::vec2(8.0F, 0.0F), {14.0F, 18.0F}});
    Gameplay game(level);
    (void)game.update(0, false);
    expect(game, 0, 2, 0, 0);
    for (int tick = 0; tick < 120; ++tick) {
        (void)game.update(0, false);
        expect(game, 0, 0, 0, 0);
    }
    bool won = false;
    for (int tick = 0; tick < 200; ++tick) {
        if (game.update(1, false) == GameplayEvent::Won) {
            expect(game, 0, 0, 0, 1);
            won = true;
            break;
        }
        expect(game, 0, 0, 0, 0);
    }
    require(won, "Fixture never reached the unlocked goal.");
    for (int tick = 0; tick < 1000; ++tick) {
        (void)game.update(1, true);
        expect(game, 0, 0, 0, 0);
    }
    game.restart();
    expect(game, 0, 0, 0, 0);
    require(game.progression().collectedCount() == 0, "Restart did not clear progression.");
    (void)game.update(0, false); // Restored flowers can legitimately collect again.
    expect(game, 0, 2, 0, 0);

    level.goalZone.position = level.spawnPosition;
    Gameplay simultaneous(level);
    (void)simultaneous.update(0, false);
    expect(simultaneous, 0, 2, 0, 1);
    simultaneous.restart(); // Even immediately after winning, restart itself has no cues.
    expect(simultaneous, 0, 0, 0, 0);
}

void damageAndFall(bool enemy)
{
    auto level = fixture();
    if (enemy) {
        level.enemies.push_back({{0.0F, 12.0F}, encounters::enemySize, -8.0F, 8.0F, encounters::enemySpeed, -1});
    } else {
        level.hazards.push_back({{0.0F, 6.0F}, encounters::hazardSize});
    }
    Gameplay game(level);
    bool failed = false;
    for (int tick = 0; tick < 100; ++tick) {
        if (game.update(1, false) != GameplayEvent::None) {
            expect(game, 0, 0, 1, 0);
            require(game.transitions().outcome == (enemy ? GameplayEvent::EnemyContact : GameplayEvent::HazardContact),
                "Wrong contact outcome.");
            failed = true;
            break;
        }
        expect(game, 0, 0, 0, 0);
    }
    require(failed && game.player().position() == level.spawnPosition, "Contact did not respawn.");
    for (int tick = 0; tick < 1000; ++tick) {
        (void)game.update(0, false);
        expect(game, 0, 0, 0, 0);
    }
    game.restart();
    expect(game, 0, 0, 0, 0);

    auto empty = fixture();
    empty.platforms.clear();
    Gameplay falling(empty);
    bool fell = false;
    for (int tick = 0; tick < 200; ++tick) {
        const auto outcome = falling.update(0, false);
        expect(falling, 0, 0, 0, 0);
        if (outcome == GameplayEvent::Fell) { fell = true; break; }
    }
    require(fell, "Silent fall fixture never fell.");
}
} // namespace

int main()
{
    try {
        jumps();
        pickupsAndWin();
        damageAndFall(true);
        damageAndFall(false);
        std::cout << "Accepted/rejected jumps, multiple/duplicate pickups, damage, silent falls, exact-once win and restart cues passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

#pragma once

#include "EnemySystem.hpp"
#include "LevelGenerator.hpp"
#include "Player.hpp"
#include "Progression.hpp"
#include <cstddef>
#include <span>

namespace cavernbloom {

enum class GameplayEvent { None, Fell, EnemyContact, HazardContact, Won };

struct GameplayTransitions {
    bool jumped = false;
    std::size_t flowersCollected = 0;
    GameplayEvent outcome = GameplayEvent::None;
};

[[nodiscard]] GameplayEvent detectContact(const Platform& playerBounds,
    std::span<const Enemy> enemies, std::span<const Hazard> hazards) noexcept;

// The immutable generated level must outlive this session. Camera and OpenGL stay in Game.
class Gameplay final {
public:
    explicit Gameplay(const GeneratedLevel& level);
    Gameplay(GeneratedLevel&&) = delete;
    [[nodiscard]] GameplayEvent update(int horizontalDirection, bool jumpPressed) noexcept;
    void restart() noexcept;
    [[nodiscard]] const Player& player() const noexcept { return player_; }
    [[nodiscard]] const Progression& progression() const noexcept { return progression_; }
    [[nodiscard]] std::span<const Enemy> enemies() const noexcept { return enemies_.enemies(); }
    // Events from the most recent step; update and restart clear them first.
    [[nodiscard]] const GameplayTransitions& transitions() const noexcept { return transitions_; }

private:
    void respawn() noexcept;
    const GeneratedLevel& level_;
    Player player_;
    Progression progression_;
    EnemySystem enemies_;
    GameplayTransitions transitions_;
};

} // namespace cavernbloom

#pragma once

#include "EnemySystem.hpp"
#include "LevelGenerator.hpp"
#include "Player.hpp"
#include "Progression.hpp"
#include <span>

namespace cavernbloom {

enum class GameplayEvent { None, Fell, EnemyContact, HazardContact, Won };

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

private:
    void respawn() noexcept;
    const GeneratedLevel& level_;
    Player player_;
    Progression progression_;
    EnemySystem enemies_;
};

} // namespace cavernbloom

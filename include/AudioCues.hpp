#pragma once

#include <array>
#include <cstddef>

namespace cavernbloom {

struct GameplayTransitions;

enum class SoundEffect { Jump, Collect, Damage, Win };

struct AudioCue {
    SoundEffect effect;
    std::size_t count;
};

// Counts preserve multiple pickups in one step without allocating a cue queue.
[[nodiscard]] std::array<AudioCue, 4> audioCues(const GameplayTransitions& transitions) noexcept;

} // namespace cavernbloom

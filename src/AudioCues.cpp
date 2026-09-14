#include "AudioCues.hpp"
#include "Gameplay.hpp"

namespace cavernbloom {

std::array<AudioCue, 4> audioCues(const GameplayTransitions& transitions) noexcept
{
    const bool damaged = transitions.outcome == GameplayEvent::EnemyContact
        || transitions.outcome == GameplayEvent::HazardContact;
    return {{{SoundEffect::Jump, transitions.jumped ? 1U : 0U},
             {SoundEffect::Collect, transitions.flowersCollected},
             {SoundEffect::Damage, damaged ? 1U : 0U},
             {SoundEffect::Win, transitions.outcome == GameplayEvent::Won ? 1U : 0U}}};
}

} // namespace cavernbloom

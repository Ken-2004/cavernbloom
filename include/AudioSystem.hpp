#pragma once

#include "AudioCues.hpp"

#include <filesystem>
#include <memory>

namespace cavernbloom {

class AudioSystem final {
public:
    explicit AudioSystem(const std::filesystem::path& assetDirectory) noexcept;
    ~AudioSystem();
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
    AudioSystem(AudioSystem&&) = delete;
    AudioSystem& operator=(AudioSystem&&) = delete;

    void play(SoundEffect effect) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace cavernbloom

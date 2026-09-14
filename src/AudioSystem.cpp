#include "AudioSystem.hpp"

#include <miniaudio.h>

#include <array>
#include <cstdio>
#include <exception>
#include <string>

namespace cavernbloom {
namespace {
constexpr float masterVolume = 0.6F;
// Per-effect gains multiply the master; pickup headroom accommodates overlapping flowers.
constexpr std::array<float, 4> effectVolumes{1.0F, 0.75F, 1.0F, 0.75F};
constexpr std::array<const char*, 4> filenames{"jump.ogg", "collect.ogg", "damage.ogg", "win.ogg"};

void report(const char* operation, const char* detail, ma_result result) noexcept
{
    std::fprintf(stderr, "Audio: %s [%s]: %s (%d). Continuing without this audio.\n",
        operation, detail, ma_result_description(result), static_cast<int>(result));
}
} // namespace

struct AudioSystem::Impl {
    ma_engine engine{};
    std::array<ma_sound_group, 4> groups{};
    std::array<ma_sound, 4> cachedSounds{};
    std::array<std::string, 4> paths;
    std::array<bool, 4> loaded{};
    std::array<bool, 4> enabled{};
    std::array<bool, 4> groupsReady{};
    bool engineReady = false;

    ~Impl()
    {
        for (std::size_t index = 0; index < loaded.size(); ++index) {
            if (loaded[index]) { ma_sound_uninit(&cachedSounds[index]); }
            if (groupsReady[index]) { ma_sound_group_uninit(&groups[index]); }
        }
        if (engineReady) { ma_engine_uninit(&engine); }
    }

    void initialize(const std::filesystem::path& directory)
    {
        const ma_result result = ma_engine_init(nullptr, &engine);
        if (result != MA_SUCCESS) {
            report("device initialization failed", "all effects disabled", result);
            return;
        }
        engineReady = true;
        ma_engine_set_volume(&engine, masterVolume);
        std::size_t loadedCount = 0;
        for (std::size_t index = 0; index < filenames.size(); ++index) {
            paths[index] = (directory / filenames[index]).string();
            const ma_result loadResult = ma_sound_init_from_file(&engine, paths[index].c_str(),
                MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, nullptr, &cachedSounds[index]);
            if (loadResult != MA_SUCCESS) {
                report("could not load sound", paths[index].c_str(), loadResult);
                continue;
            }
            loaded[index] = true;
            const ma_result groupResult = ma_sound_group_init(&engine, 0, nullptr, &groups[index]);
            if (groupResult != MA_SUCCESS) {
                report("volume group initialization failed", filenames[index], groupResult);
                continue;
            }
            groupsReady[index] = true;
            ma_sound_group_set_volume(&groups[index], effectVolumes[index]);
            enabled[index] = true;
            ++loadedCount;
        }
        std::fprintf(stdout, "Audio: loaded %zu/4 effects; master volume %.2f.\n",
            loadedCount, static_cast<double>(masterVolume));
    }
};

AudioSystem::AudioSystem(const std::filesystem::path& assetDirectory) noexcept
{
    try {
        impl_ = std::make_unique<Impl>();
        impl_->initialize(assetDirectory);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Audio: initialization failed: %s. Continuing silently.\n", error.what());
        impl_.reset();
    }
}

AudioSystem::~AudioSystem() = default;

void AudioSystem::play(SoundEffect effect) noexcept
{
    const std::size_t index = static_cast<std::size_t>(effect);
    if (!impl_ || !impl_->engineReady || index >= filenames.size() || !impl_->enabled[index]) { return; }

    // The engine shares predecoded data, owns overlapping one-shots, and recycles finished voices.
    const ma_result result = ma_engine_play_sound(&impl_->engine, impl_->paths[index].c_str(),
        &impl_->groups[index]);
    if (result != MA_SUCCESS) {
        report("playback failed", impl_->paths[index].c_str(), result);
        impl_->enabled[index] = false; // Avoid repeating diagnostics on every later event.
    }
}

} // namespace cavernbloom

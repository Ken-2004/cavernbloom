#include "GamePresentation.hpp"
#include "Camera2D.hpp"
#include "Gameplay.hpp"
#include "PresentationMath.hpp"
#include "Renderer.hpp"
#include "VisualTheme.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cavernbloom {
namespace {

constexpr float diagonalAngle = 0.785398163F;

void background(const Renderer& r, glm::vec2 view, float cameraX)
{
    constexpr int bandCount = 12;
    const float bandHeight = view.y / static_cast<float>(bandCount);
    for (int band = 0; band < bandCount; ++band) {
        const float depth = 1.0F - (static_cast<float>(band) + 0.5F) / static_cast<float>(bandCount);
        r.drawRectangle({view.x * 0.5F, (static_cast<float>(band) + 0.5F) * bandHeight},
                        {view.x, bandHeight + 0.5F}, glm::mix(theme::background, theme::atmosphere, depth * depth));
    }
    constexpr std::array<float, 4> heights{240.0F, 330.0F, 190.0F, 280.0F};
    constexpr std::array<float, 3> factors{0.12F, 0.25F, 0.40F};
    for (std::size_t layer = 0; layer < factors.size(); ++layer) {
        // Adapt spacing for unusually wide views, keeping each layer below 16 motifs.
        const float spacing = std::max(280.0F + static_cast<float>(layer) * 40.0F, view.x / 10.0F);
        const float offset = presentation::parallaxOffset(cameraX, factors[layer], spacing * 4.0F);
        const int count = static_cast<int>(std::ceil(view.x / spacing)) + 5;
        for (int index = 0; index < count; ++index) {
            const float x = static_cast<float>(index) * spacing - offset;
            if (x < -spacing || x > view.x + spacing) { continue; }
            const float height = heights[static_cast<std::size_t>(index) % heights.size()];
            if (layer == 0) {
                r.drawRectangle({x, height * 0.5F}, {92.0F, height}, theme::farStone, -0.13F);
                r.drawRectangle({x + 36.0F, height * 0.38F}, {48.0F, height * 0.76F}, theme::farFacet, 0.16F);
            } else if (layer == 1) {
                r.drawRectangle({x + 100.0F, height * 0.48F}, {18.0F, height * 0.96F}, theme::forest, 0.06F);
                r.drawRectangle({x + 72.0F, height * 0.65F}, {12.0F, 105.0F}, theme::forest, 0.65F);
                r.drawRectangle({x + 126.0F, height * 0.80F}, {9.0F, 82.0F}, theme::forest, -0.55F);
            } else {
                r.drawRectangle({x, view.y - height * 0.23F}, {150.0F, height * 0.54F}, theme::nearStone, 0.13F);
                r.drawRectangle({x + 53.0F, view.y - height * 0.25F}, {44.0F, height * 0.65F}, theme::nearStone, -0.16F);
            }
        }
    }
    const float period = view.x + 40.0F;
    const float offset = presentation::parallaxOffset(cameraX, 0.18F, period);
    for (int index = 0; index < 16; ++index) {
        const float x = std::fmod(static_cast<float>(index) * period / 16.0F + period - offset, period) - 20.0F;
        const float y = 130.0F + static_cast<float>((index * 73) % 360);
        r.drawRectangle({x, y}, {2.5F, 2.5F}, theme::mote, diagonalAngle);
    }
}

void platform(const Renderer& r, const Platform& p, bool finalPlatform)
{
    const float top = p.position.y + p.size.y * 0.5F;
    r.drawRectangle(p.position, p.size, theme::stone);
    r.drawRectangle({p.position.x, p.position.y - p.size.y * 0.5F + 2.0F}, {p.size.x, 4.0F}, theme::stoneShadow);
    r.drawRectangle({p.position.x, top - 1.5F}, {p.size.x, 3.0F}, finalPlatform ? theme::gold : theme::moss);
    r.drawRectangle(p.position + glm::vec2(-p.size.x * 0.23F, -1.0F), {2.0F, p.size.y - 8.0F}, theme::stoneSeam);
    r.drawRectangle({p.position.x + p.size.x * 0.20F, top - 4.0F}, {p.size.x * 0.20F, 2.0F}, theme::moss);
}

void flower(const Renderer& r, glm::vec2 center, glm::vec2 size, bool bright, bool glow)
{
    const glm::vec4 petals = bright ? theme::petal : theme::inactive;
    const glm::vec2 bloom = center + glm::vec2(0.0F, size.y * 0.17F);
    if (glow && bright) {
        r.drawRectangle(bloom, size * 1.65F, theme::flowerGlow, diagonalAngle);
    }
    r.drawRectangle(center - glm::vec2(0.0F, size.y * 0.20F), {size.x * 0.14F, size.y * 0.60F},
                    bright ? theme::stem : theme::inactive);
    r.drawRectangle(center + glm::vec2(size.x * 0.18F, -size.y * 0.20F), size * glm::vec2(0.35F, 0.12F),
                    bright ? theme::stem : theme::inactive, 0.45F);
    for (const glm::vec2 direction : {glm::vec2(-1.0F, 0.0F), glm::vec2(1.0F, 0.0F),
                                     glm::vec2(0.0F, -1.0F), glm::vec2(0.0F, 1.0F)}) {
        r.drawRectangle(bloom + direction * size * glm::vec2(0.30F, 0.24F),
                        size * glm::vec2(0.35F, 0.24F), petals);
    }
    r.drawRectangle(bloom, size * glm::vec2(0.25F, 0.20F), bright ? theme::gold : theme::panelBorder);
}

void player(const Renderer& r, const Player& p, int facing)
{
    const glm::vec2 center = p.position();
    const glm::vec2 size = p.size();
    const float direction = static_cast<float>(facing);
    r.drawRectangle(center + size * glm::vec2(0.0F, -0.10F), size * glm::vec2(0.75F, 0.59F), theme::playerShade);
    for (const float side : {-1.0F, 1.0F}) {
        r.drawRectangle(center + size * glm::vec2(side * 0.20F, -0.4375F), size * glm::vec2(0.30F, 0.125F), theme::face);
    }
    r.drawRectangle(center + size * glm::vec2(0.0F, 0.3125F), size * glm::vec2(0.90F, 0.375F), theme::player);
    r.drawRectangle(center + size * glm::vec2(direction * 0.08F, 0.30F), size * glm::vec2(0.58F, 0.20F), theme::face);
    r.drawRectangle(center + size * glm::vec2(direction * 0.23F, 0.32F), size * glm::vec2(0.12F, 0.08F), theme::light);
    r.drawRectangle(center + size * glm::vec2(0.0F, 0.09F), size * glm::vec2(0.80F, 0.075F), theme::petalLight);
    r.drawRectangle(center + size * glm::vec2(-direction * 0.30F, -0.06F), size * glm::vec2(0.15F, 0.26F), theme::petal);
    r.drawRectangle(center + size * glm::vec2(direction * 0.31F, -0.18F), size * glm::vec2(0.12F, 0.17F), theme::player);
}

void enemy(const Renderer& r, const Enemy& e)
{
    const float facing = static_cast<float>(e.direction);
    r.drawRectangle(e.position - glm::vec2(0.0F, e.size.y * 0.12F), e.size * glm::vec2(1.0F, 0.76F), theme::enemyShade);
    r.drawRectangle(e.position, e.size * glm::vec2(1.0F, 0.58F), theme::enemy);
    for (const float side : {-1.0F, 1.0F}) {
        r.drawRectangle(e.position + e.size * glm::vec2(side * 0.32F, 0.34F), e.size * glm::vec2(0.17F, 0.27F), theme::horn, -side * 0.28F);
        r.drawRectangle(e.position + e.size * glm::vec2(side * 0.22F + facing * 0.08F, 0.04F),
                        e.size * glm::vec2(0.15F, 0.12F), theme::light);
    }
}

void hazard(const Renderer& r, const Hazard& h)
{
    r.drawRectangle(h.position - glm::vec2(0.0F, h.size.y * 0.35F), h.size * glm::vec2(1.0F, 0.30F), theme::thornBase);
    for (int index = -1; index <= 1; ++index) {
        const float x = static_cast<float>(index);
        const glm::vec2 stem = h.position + glm::vec2(x * h.size.x * 0.29F, 0.0F);
        r.drawRectangle(stem, h.size * glm::vec2(0.14F, 0.83F), theme::thorn, -x * 0.22F);
        r.drawRectangle(stem + glm::vec2(x, h.size.y * 0.26F), h.size * glm::vec2(0.12F, 0.25F), theme::thornTip, diagonalAngle);
    }
}

void goal(const Renderer& r, const Platform& zone, bool unlocked)
{
    const glm::vec4 accent = unlocked ? theme::gold : theme::locked;
    const glm::vec2 center = zone.position;
    if (unlocked) {
        r.drawRectangle(center, zone.size + glm::vec2(44.0F, 24.0F), theme::portalGlow);
        r.drawRectangle(center, zone.size + glm::vec2(24.0F, 12.0F), theme::portalGlow);
    }
    r.drawRectangle(center, zone.size, unlocked ? theme::forest : theme::lockedCore);
    for (const float side : {-1.0F, 1.0F}) {
        r.drawRectangle(center + glm::vec2(side * 14.0F, -3.0F), {5.0F, 62.0F}, theme::stoneSeam);
        r.drawRectangle(center + glm::vec2(side * 14.0F, 0.0F), {2.0F, 48.0F}, accent);
        r.drawRectangle(center + glm::vec2(side * 8.0F, 31.0F), {5.0F, 23.0F}, accent, side * diagonalAngle);
        if (unlocked) {
            r.drawRectangle(center + glm::vec2(side * 28.0F, 12.0F), {6.0F, 6.0F}, theme::unlocked, diagonalAngle);
            r.drawRectangle(center + glm::vec2(side * 23.0F, -14.0F), {3.0F, 3.0F}, theme::gold, diagonalAngle);
        }
    }
    r.drawRectangle(center + glm::vec2(0.0F, -36.0F), {42.0F, 5.0F}, accent);
    if (unlocked) {
        flower(r, center + glm::vec2(0.0F, 9.0F), {14.0F, 22.0F}, true, false);
        r.drawRectangle(center - glm::vec2(0.0F, 15.0F), {3.0F, 18.0F}, theme::unlocked);
    } else {
        r.drawRectangle(center, {17.0F, 3.0F}, accent, diagonalAngle);
        r.drawRectangle(center, {17.0F, 3.0F}, accent, -diagonalAngle);
    }
}

void hud(const Renderer& r, glm::vec2 view, const Progression& progress)
{
    const auto layout = presentation::hudLayout(view, progress.totalCount());
    r.drawRectangle(layout.center, layout.size, theme::panelBorder);
    r.drawRectangle(layout.center, layout.size - glm::vec2(2.0F * layout.scale), theme::panel);
    for (std::size_t index = 0; index < progress.totalCount(); ++index) {
        flower(r, layout.firstMarker + glm::vec2(static_cast<float>(index) * layout.markerStep, 0.0F),
                glm::vec2(14.0F, 22.0F) * layout.scale, index < progress.collectedCount(), false);
    }
}

void winPanel(const Renderer& r, glm::vec2 view)
{
    r.drawRectangle(view * 0.5F, view, theme::dimmer);
    const glm::vec2 center = view * 0.5F;
    const float scale = std::min({1.0F, view.x * 0.9F / 440.0F, view.y * 0.8F / 280.0F});
    r.drawRectangle(center, glm::vec2(444.0F, 284.0F) * scale, theme::panelBorder);
    r.drawRectangle(center, glm::vec2(440.0F, 280.0F) * scale, theme::panel);
    for (const float side : {-1.0F, 1.0F}) {
        r.drawRectangle(center + glm::vec2(side * 130.0F, 30.0F) * scale, glm::vec2(64.0F, 2.0F) * scale, theme::gold);
        r.drawRectangle(center + glm::vec2(side * 172.0F, 30.0F) * scale, glm::vec2(6.0F) * scale, theme::unlocked, diagonalAngle);
    }
    r.drawRectangle(center + glm::vec2(0.0F, 34.0F) * scale, glm::vec2(85.0F) * scale, theme::portalGlow, diagonalAngle);
    flower(r, center + glm::vec2(0.0F, 30.0F) * scale, glm::vec2(48.0F, 70.0F) * scale, true, true);
    r.drawRectangle(center + glm::vec2(-8.0F, -70.0F) * scale, glm::vec2(22.0F, 6.0F) * scale, theme::unlocked, -diagonalAngle);
    r.drawRectangle(center + glm::vec2(10.0F, -62.0F) * scale, glm::vec2(37.0F, 6.0F) * scale, theme::unlocked, diagonalAngle);
    r.drawRectangle(center + glm::vec2(0.0F, -110.0F) * scale, glm::vec2(240.0F, 2.0F) * scale, theme::gold);
}

} // namespace

void GamePresentation::face(int horizontalDirection) noexcept
{
    if (horizontalDirection != 0) { facing_ = horizontalDirection < 0 ? -1 : 1; }
}

void GamePresentation::render(const Renderer& renderer, const Camera2D& camera,
                              const GeneratedLevel& level, const Gameplay& gameplay) const noexcept
{
    const glm::vec2 view = camera.visibleSize();
    const glm::mat4 screen = glm::ortho(0.0F, view.x, 0.0F, view.y, -1.0F, 1.0F);
    renderer.beginFrame(screen, theme::background);
    background(renderer, view, camera.center().x);
    renderer.setViewProjection(camera.viewProjection());
    const auto visible = [&](glm::vec2 position, glm::vec2 size) {
        return std::abs(position.x - camera.center().x) <= (view.x + size.x) * 0.5F + 40.0F;
    };
    for (std::size_t index = 0; index < level.platforms.size(); ++index) {
        const Platform& p = level.platforms[index];
        if (visible(p.position, p.size)) { platform(renderer, p, index == level.goalPlatformIndex); }
    }
    for (const Enemy& e : gameplay.enemies()) {
        if (visible(e.position, e.size)) { enemy(renderer, e); }
    }
    for (const Hazard& h : level.hazards) {
        if (visible(h.position, h.size)) { hazard(renderer, h); }
    }
    for (const Collectible& c : gameplay.progression().collectibles()) {
        if (!c.collected && visible(c.position, c.size)) { flower(renderer, c.position, c.size, true, true); }
    }
    if (visible(level.goalZone.position, level.goalZone.size)) {
        goal(renderer, level.goalZone, gameplay.progression().allCollected());
    }
    player(renderer, gameplay.player(), facing_);
    renderer.setViewProjection(screen);
    if (gameplay.progression().state() == GameState::Won) { winPanel(renderer, view); }
    hud(renderer, view, gameplay.progression());
}

} // namespace cavernbloom

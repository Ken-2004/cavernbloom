#pragma once

namespace cavernbloom {

class Renderer;
class Camera2D;
class Gameplay;
struct GeneratedLevel;

class GamePresentation final {
public:
    void face(int horizontalDirection) noexcept;
    void resetFacing() noexcept { facing_ = 1; }
    void render(const Renderer& renderer, const Camera2D& camera,
                const GeneratedLevel& level, const Gameplay& gameplay) const noexcept;

private:
    int facing_ = 1;
};

} // namespace cavernbloom

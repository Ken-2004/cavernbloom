#pragma once

#include "Player.hpp"
#include "LevelGenerator.hpp"
#include "Camera2D.hpp"
#include "Gameplay.hpp"
#include "GamePresentation.hpp"
#include "AudioSystem.hpp"

#include <memory>
#include <filesystem>

struct GLFWwindow;

namespace cavernbloom {

class Renderer;

class Game final {
public:
    explicit Game(const std::filesystem::path& resourceRoot);
    ~Game();
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    void run();

private:
    struct GlfwLifetime {
        GlfwLifetime();
        ~GlfwLifetime();
    };
    struct WindowDeleter {
        void operator()(GLFWwindow* window) const noexcept;
    };

    void processInput();
    void update();
    void render();
    void restart();
    void updateWindowTitle();

    // Reverse destruction order keeps the context alive for all GPU cleanup.
    GlfwLifetime glfw_;
    std::unique_ptr<GLFWwindow, WindowDeleter> window_;
    std::unique_ptr<Renderer> renderer_;
    GeneratedLevel level_ = generateLevel(generation::developmentSeed);
    Gameplay gameplay_{level_};
    Camera2D camera_;
    GamePresentation presentation_;
    std::unique_ptr<AudioSystem> audio_;
    int horizontalDirection_ = 0;
    bool jumpRequested_ = false;
    bool restartRequested_ = false;
};

} // namespace cavernbloom

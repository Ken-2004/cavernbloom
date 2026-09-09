#pragma once

#include "Player.hpp"
#include "LevelGenerator.hpp"

#include <memory>

struct GLFWwindow;

namespace cavernbloom {

class Renderer;

class Game final {
public:
    Game();
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
    void update(float deltaSeconds);
    void render();

    // Reverse destruction order keeps the context alive for all GPU cleanup.
    GlfwLifetime glfw_;
    std::unique_ptr<GLFWwindow, WindowDeleter> window_;
    std::unique_ptr<Renderer> renderer_;
    GeneratedLevel level_ = generateLevel(generation::developmentSeed);
    Player player_{level_.spawnPosition};
    int horizontalDirection_ = 0;
    bool jumpRequested_ = false;
};

} // namespace cavernbloom

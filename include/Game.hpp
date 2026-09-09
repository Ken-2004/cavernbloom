#pragma once

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
    void update(double deltaSeconds);
    void render();

    // Reverse destruction order keeps the context alive for all GPU cleanup.
    GlfwLifetime glfw_;
    std::unique_ptr<GLFWwindow, WindowDeleter> window_;
    std::unique_ptr<Renderer> renderer_;
};

} // namespace cavernbloom

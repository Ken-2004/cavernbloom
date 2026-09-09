#include "Game.hpp"
#include "Renderer.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace cavernbloom {

Game::GlfwLifetime::GlfwLifetime()
{
    glfwSetErrorCallback([](int code, const char* description) {
        std::cerr << "GLFW error " << code << ": " << description << '\n';
    });
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW.");
    }
}

Game::GlfwLifetime::~GlfwLifetime()
{
    glfwTerminate();
}

void Game::WindowDeleter::operator()(GLFWwindow* window) const noexcept
{
    glfwDestroyWindow(window);
}

Game::Game()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    window_.reset(glfwCreateWindow(1280, 720, "CavernBloom", nullptr, nullptr));
    if (!window_) {
        throw std::runtime_error("Failed to create an OpenGL 3.3 Core window.");
    }
    glfwMakeContextCurrent(window_.get());
    if (gladLoadGL(glfwGetProcAddress) == 0 || !GLAD_GL_VERSION_3_3) {
        throw std::runtime_error("Failed to load OpenGL 3.3 functions with GLAD.");
    }
    glfwSwapInterval(1);
    renderer_ = std::make_unique<Renderer>(CAVERNBLOOM_SHADER_DIR);

    glfwSetWindowUserPointer(window_.get(), this);
    glfwSetFramebufferSizeCallback(window_.get(), [](GLFWwindow* window, int width, int height) {
        auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
        game->renderer_->resize(width, height);
    });
    glfwSetKeyCallback(window_.get(), [](GLFWwindow* window, int key, int, int action, int) {
        auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            game->jumpRequested_ = true;
        }
    });
    glfwSetWindowFocusCallback(window_.get(), [](GLFWwindow* window, int focused) {
        if (focused == GLFW_FALSE) {
            auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
            game->jumpRequested_ = false;
        }
    });
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window_.get(), &width, &height);
    renderer_->resize(width, height);
    std::cout << "CavernBloom | OpenGL " << glGetString(GL_VERSION)
              << " | A/D or arrows to move | Space to jump | Escape to close\n";
}

Game::~Game()
{
    glfwSetFramebufferSizeCallback(window_.get(), nullptr);
    glfwSetKeyCallback(window_.get(), nullptr);
    glfwSetWindowFocusCallback(window_.get(), nullptr);
    glfwSetWindowUserPointer(window_.get(), nullptr);
    glfwMakeContextCurrent(window_.get());
}

void Game::run()
{
    double previousTime = glfwGetTime();
    double accumulator = 0.0;
    while (glfwWindowShouldClose(window_.get()) == GLFW_FALSE) {
        processInput();
        if (glfwWindowShouldClose(window_.get()) == GLFW_TRUE) {
            break;
        }
        const double currentTime = glfwGetTime();
        const double frameDelta = std::clamp(currentTime - previousTime,
                                             0.0, simulation::maxFrameDeltaSeconds);
        previousTime = currentTime;
        accumulator += frameDelta;
        while (accumulator >= simulation::fixedStepSeconds) {
            update(static_cast<float>(simulation::fixedStepSeconds));
            accumulator -= simulation::fixedStepSeconds;
        }
        render();
        glfwSwapBuffers(window_.get());
        if (glfwGetWindowAttrib(window_.get(), GLFW_ICONIFIED) == GLFW_TRUE) {
            glfwWaitEventsTimeout(0.05);
        }
    }
}

void Game::processInput()
{
    glfwPollEvents();
    if (glfwGetKey(window_.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_.get(), GLFW_TRUE);
    }
    const bool left = glfwGetKey(window_.get(), GLFW_KEY_A) == GLFW_PRESS
                      || glfwGetKey(window_.get(), GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool right = glfwGetKey(window_.get(), GLFW_KEY_D) == GLFW_PRESS
                       || glfwGetKey(window_.get(), GLFW_KEY_RIGHT) == GLFW_PRESS;
    horizontalDirection_ = static_cast<int>(right) - static_cast<int>(left);
}

void Game::update(float deltaSeconds)
{
    // Keep a press through frames without a simulation step; consume it only once.
    player_.update(deltaSeconds, horizontalDirection_, std::exchange(jumpRequested_, false));
}

void Game::render()
{
    renderer_->beginFrame();
    renderer_->drawRectangle(
        glm::vec2(0.0F, simulation::floorTop - simulation::floorThickness * 0.5F),
        glm::vec2(renderer_->viewWidth(), simulation::floorThickness),
        glm::vec4(0.25F, 0.32F, 0.40F, 1.0F));
    renderer_->drawRectangle(player_.position(), player_.size(),
                             glm::vec4(0.35F, 0.85F, 0.65F, 1.0F));
}

} // namespace cavernbloom

#include "Game.hpp"
#include "Renderer.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

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
    camera_.setWorldBounds(level_.bounds);
    camera_.recenter(player_.position());

    glfwSetWindowUserPointer(window_.get(), this);
    glfwSetFramebufferSizeCallback(window_.get(), [](GLFWwindow* window, int width, int height) {
        auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
        game->renderer_->resize(width, height);
        game->camera_.setViewportSize(width, height);
    });
    glfwSetKeyCallback(window_.get(), [](GLFWwindow* window, int key, int, int action, int) {
        auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            game->jumpRequested_ = true;
        }
        if (key == GLFW_KEY_R && action == GLFW_PRESS) {
            game->restartRequested_ = true;
        }
    });
    glfwSetWindowFocusCallback(window_.get(), [](GLFWwindow* window, int focused) {
        if (focused == GLFW_FALSE) {
            auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
            game->jumpRequested_ = false;
            game->restartRequested_ = false;
        }
    });
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window_.get(), &width, &height);
    renderer_->resize(width, height);
    camera_.setViewportSize(width, height);
    std::cout << "CavernBloom | OpenGL " << glGetString(GL_VERSION)
              << " | Level seed " << level_.seed
              << " | A/D or arrows to move | Space to jump | R to restart | Escape to close\n";
    updateWindowTitle();
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
        camera_.follow(player_.position());
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
    if (std::exchange(restartRequested_, false)) {
        restart();
    }
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
    if (progression_.state() == GameState::Won) {
        jumpRequested_ = false;
        return;
    }
    // Keep a press through frames without a simulation step; consume it only once.
    player_.update(deltaSeconds, horizontalDirection_, std::exchange(jumpRequested_, false),
                   level_.platforms);
    if (player_.position().y < simulation::fallResetY) {
        player_.resetToSpawn();
        camera_.recenter(player_.position());
    }
    const std::size_t previousCount = progression_.collectedCount();
    const bool justWon = progression_.update({player_.position(), player_.size()});
    if (justWon) {
        std::cout << "All flowers collected. Level won! Press R to restart the same seed.\n";
    }
    if (justWon || progression_.collectedCount() != previousCount) {
        updateWindowTitle();
    }
}

void Game::restart()
{
    player_.resetToSpawn();
    camera_.recenter(player_.position());
    progression_.reset();
    jumpRequested_ = false;
    updateWindowTitle();
}

void Game::updateWindowTitle()
{
    const std::string title = "CavernBloom | Flowers " + std::to_string(progression_.collectedCount())
        + "/" + std::to_string(progression_.totalCount())
        + (progression_.state() == GameState::Won ? " | Won - R to restart" : " | Playing - R to restart");
    glfwSetWindowTitle(window_.get(), title.c_str());
}

void Game::render()
{
    renderer_->beginFrame(camera_.viewProjection());
    for (std::size_t index = 0; index < level_.platforms.size(); ++index) {
        const Platform& platform = level_.platforms[index];
        const glm::vec4 color = index == level_.goalPlatformIndex
            ? glm::vec4(0.95F, 0.68F, 0.25F, 1.0F) : glm::vec4(0.25F, 0.32F, 0.40F, 1.0F);
        renderer_->drawRectangle(platform.position, platform.size,
                                 color);
    }
    const glm::vec4 flowerColor(1.0F, 0.25F, 0.70F, 1.0F);
    const glm::vec4 goalColor = progression_.allCollected()
        ? glm::vec4(0.30F, 1.0F, 0.35F, 1.0F) : glm::vec4(0.65F, 0.28F, 0.22F, 1.0F);
    renderer_->drawRectangle(level_.goalZone.position, level_.goalZone.size, goalColor);
    for (const Collectible& collectible : progression_.collectibles()) {
        if (!collectible.collected) {
            renderer_->drawRectangle(collectible.position, collectible.size, flowerColor);
        }
    }
    renderer_->drawRectangle(player_.position(), player_.size(),
                             glm::vec4(0.35F, 0.85F, 0.65F, 1.0F));

    const glm::vec2 viewSize = camera_.visibleSize();
    renderer_->setViewProjection(glm::ortho(0.0F, viewSize.x, 0.0F, viewSize.y, -1.0F, 1.0F));
    for (std::size_t index = 0; index < progression_.totalCount(); ++index) {
        const glm::vec4 color = index < progression_.collectedCount()
            ? flowerColor : glm::vec4(0.18F, 0.20F, 0.26F, 1.0F);
        renderer_->drawRectangle({24.0F + static_cast<float>(index) * 22.0F, viewSize.y - 24.0F},
                                 {14.0F, 14.0F}, color);
    }
    if (progression_.state() == GameState::Won) {
        const glm::vec2 center = viewSize * 0.5F;
        renderer_->drawRectangle(center, {320.0F, 96.0F}, {0.08F, 0.30F, 0.24F, 1.0F});
        renderer_->drawRectangle(center, {272.0F, 24.0F}, goalColor);
    }
}

} // namespace cavernbloom

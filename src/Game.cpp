#include "Game.hpp"
#include "Renderer.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

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

    glfwSetWindowUserPointer(window_.get(), renderer_.get());
    glfwSetFramebufferSizeCallback(window_.get(), [](GLFWwindow* window, int width, int height) {
        auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
        renderer->resize(width, height);
    });
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window_.get(), &width, &height);
    renderer_->resize(width, height);
    std::cout << "CavernBloom | OpenGL " << glGetString(GL_VERSION)
              << " | Escape to close\n";
}

Game::~Game()
{
    glfwSetFramebufferSizeCallback(window_.get(), nullptr);
    glfwSetWindowUserPointer(window_.get(), nullptr);
    glfwMakeContextCurrent(window_.get());
}

void Game::run()
{
    double previousTime = glfwGetTime();
    while (glfwWindowShouldClose(window_.get()) == GLFW_FALSE) {
        const double currentTime = glfwGetTime();
        const double deltaSeconds = currentTime - previousTime;
        previousTime = currentTime;

        processInput();
        if (glfwWindowShouldClose(window_.get()) == GLFW_TRUE) {
            break;
        }
        update(deltaSeconds);
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
}

void Game::update([[maybe_unused]] double deltaSeconds)
{
    // The foundation scene is stationary; simulation will consume deltaSeconds later.
}

void Game::render()
{
    renderer_->render();
}

} // namespace cavernbloom

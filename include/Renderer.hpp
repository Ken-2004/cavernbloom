#pragma once

#include "Shader.hpp"

namespace cavernbloom {

class Renderer final {
public:
    explicit Renderer(const std::filesystem::path& shaderDirectory);
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void resize(int framebufferWidth, int framebufferHeight) noexcept;
    void render() const noexcept;

private:
    struct QuadGeometry {
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
        ~QuadGeometry();
    };

    Shader shader_;
    QuadGeometry quad_;
    GLint projectionLocation_ = -1;
    GLint modelLocation_ = -1;
    GLint colorLocation_ = -1;
    glm::mat4 projection_{1.0F};
    glm::mat4 model_{1.0F};
    bool drawable_ = false;
};

} // namespace cavernbloom

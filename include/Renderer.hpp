#pragma once

#include "Shader.hpp"

#include <glm/vec2.hpp>

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
    void beginFrame(const glm::mat4& viewProjection) const noexcept;
    // Call beginFrame first. Position is the center in world coordinates (+X right, +Y up).
    void drawRectangle(const glm::vec2& position, const glm::vec2& size,
                       const glm::vec4& color) const noexcept;

private:
    struct QuadGeometry {
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
        ~QuadGeometry();
    };

    Shader shader_;
    QuadGeometry quad_;
    GLint viewProjectionLocation_ = -1;
    GLint modelLocation_ = -1;
    GLint colorLocation_ = -1;
    bool drawable_ = false;
};

} // namespace cavernbloom

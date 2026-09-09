#include "Renderer.hpp"

#include <array>
#include <stdexcept>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

namespace cavernbloom {

Renderer::QuadGeometry::~QuadGeometry()
{
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
}

Renderer::Renderer(const std::filesystem::path& shaderDirectory)
    : shader_(shaderDirectory / "basic.vert", shaderDirectory / "basic.frag")
{
    viewProjectionLocation_ = shader_.uniformLocation("uViewProjection");
    modelLocation_ = shader_.uniformLocation("uModel");
    colorLocation_ = shader_.uniformLocation("uColor");

    constexpr std::array<float, 8> vertices{
        -0.5F, -0.5F, 0.5F, -0.5F, 0.5F, 0.5F, -0.5F, 0.5F
    };
    constexpr std::array<GLuint, 6> indices{0, 1, 2, 2, 3, 0};
    glGenVertexArrays(1, &quad_.vertexArray);
    glGenBuffers(1, &quad_.vertexBuffer);
    glGenBuffers(1, &quad_.indexBuffer);
    glBindVertexArray(quad_.vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, quad_.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(vertices)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(indices)),
                 indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          static_cast<GLsizei>(2 * sizeof(float)), nullptr);
    glEnableVertexAttribArray(0);
    // The index buffer remains attached to the VAO.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const GLenum error = glGetError();
    if (error != GL_NO_ERROR || quad_.vertexArray == 0
        || quad_.vertexBuffer == 0 || quad_.indexBuffer == 0) {
        throw std::runtime_error("Failed to create quad geometry; OpenGL error "
                                 + std::to_string(error));
    }
}

void Renderer::resize(int framebufferWidth, int framebufferHeight) noexcept
{
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    drawable_ = framebufferWidth > 0 && framebufferHeight > 0;
}

void Renderer::beginFrame(const glm::mat4& viewProjection) const noexcept
{
    if (!drawable_) {
        return;
    }
    glClearColor(0.035F, 0.055F, 0.075F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
    setViewProjection(viewProjection);
}

void Renderer::setViewProjection(const glm::mat4& viewProjection) const noexcept
{
    if (!drawable_) {
        return;
    }
    shader_.bind();
    shader_.setMatrix(viewProjectionLocation_, viewProjection);
}

void Renderer::drawRectangle(const glm::vec2& position, const glm::vec2& size,
                             const glm::vec4& color) const noexcept
{
    if (!drawable_) {
        return;
    }
    const glm::mat4 model = glm::scale(
        glm::translate(glm::mat4(1.0F), glm::vec3(position, 0.0F)), glm::vec3(size, 1.0F));
    shader_.setMatrix(modelLocation_, model);
    shader_.setColor(colorLocation_, color);
    glBindVertexArray(quad_.vertexArray);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace cavernbloom

#pragma once

#include <filesystem>

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace cavernbloom {

// Construction, use and destruction require the owning OpenGL context to be current.
class Shader final {
public:
    Shader(const std::filesystem::path& vertexPath,
           const std::filesystem::path& fragmentPath);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    void bind() const noexcept;
    [[nodiscard]] GLint uniformLocation(const char* name) const;
    // Bind this program before setting uniforms.
    void setMatrix(GLint location, const glm::mat4& value) const noexcept;
    void setColor(GLint location, const glm::vec4& value) const noexcept;

private:
    GLuint program_ = 0;
};

} // namespace cavernbloom

#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <glm/gtc/type_ptr.hpp>

namespace cavernbloom {
namespace {

std::string readShader(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open shader file: " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    if (file.bad()) {
        throw std::runtime_error("Cannot read shader file: " + path.string());
    }
    return contents.str();
}

struct CompiledShader final {
    GLuint id;
    explicit CompiledShader(GLenum stage) : id(glCreateShader(stage))
    {
        if (id == 0) {
            throw std::runtime_error("OpenGL could not allocate a shader object.");
        }
    }
    ~CompiledShader() { glDeleteShader(id); }
    CompiledShader(const CompiledShader&) = delete;
    CompiledShader& operator=(const CompiledShader&) = delete;

    void compile(const std::filesystem::path& path) const
    {
        const std::string source = readShader(path);
        const char* sourcePointer = source.c_str();
        glShaderSource(id, 1, &sourcePointer, nullptr);
        glCompileShader(id);
        GLint success = GL_FALSE;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            GLint length = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
            glGetShaderInfoLog(id, static_cast<GLsizei>(log.size()), nullptr, log.data());
            throw std::runtime_error("Shader compilation failed [" + path.string() + "]:\n" + log);
        }
    }
};

} // namespace

Shader::Shader(const std::filesystem::path& vertexPath,
               const std::filesystem::path& fragmentPath)
{
    const CompiledShader vertex(GL_VERTEX_SHADER);
    vertex.compile(vertexPath);
    const CompiledShader fragment(GL_FRAGMENT_SHADER);
    fragment.compile(fragmentPath);

    program_ = glCreateProgram();
    if (program_ == 0) {
        throw std::runtime_error("OpenGL could not allocate a shader program.");
    }
    try {
        glAttachShader(program_, vertex.id);
        glAttachShader(program_, fragment.id);
        glLinkProgram(program_);
        GLint success = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            GLint length = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &length);
            std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
            glGetProgramInfoLog(program_, static_cast<GLsizei>(log.size()), nullptr, log.data());
            throw std::runtime_error("Shader link failed [" + vertexPath.string() + ", "
                                     + fragmentPath.string() + "]:\n" + log);
        }
        glDetachShader(program_, vertex.id);
        glDetachShader(program_, fragment.id);
    } catch (...) {
        glDeleteProgram(program_);
        program_ = 0;
        throw;
    }
}

Shader::~Shader()
{
    glDeleteProgram(program_);
}

void Shader::bind() const noexcept
{
    glUseProgram(program_);
}

GLint Shader::uniformLocation(const char* name) const
{
    const GLint location = glGetUniformLocation(program_, name);
    if (location == -1) {
        throw std::runtime_error(std::string("Required shader uniform is missing or inactive: ") + name);
    }
    return location;
}

void Shader::setMatrix(GLint location, const glm::mat4& value) const noexcept
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setColor(GLint location, const glm::vec4& value) const noexcept
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}

} // namespace cavernbloom

#include "shader.h"
#include "asset_loader.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define SHADER_LOG(msg) emscripten_log(EM_LOG_CONSOLE, "Shader: %s", msg)
#define SHADER_ERR(msg) emscripten_log(EM_LOG_ERROR, "Shader: %s", msg)
#elif defined(__ANDROID__)
#include <android/log.h>
#define SHADER_LOG(msg) __android_log_print(ANDROID_LOG_INFO, "Shader", "%s", msg)
#define SHADER_ERR(msg) __android_log_print(ANDROID_LOG_ERROR, "Shader", "%s", msg)
#else
#define SHADER_LOG(msg) std::cout << "Shader: " << msg << std::endl
#define SHADER_ERR(msg) std::cerr << "Shader: " << msg << std::endl
#endif

namespace polarclock {

Shader::~Shader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    SHADER_LOG(("Loading: " + vertPath).c_str());

    std::string vertSource, fragSource;

    if (!AssetLoader::instance().loadTextFile(vertPath, vertSource)) {
        SHADER_ERR(("Failed to load vertex shader: " + vertPath).c_str());
        return false;
    }

    if (!AssetLoader::instance().loadTextFile(fragPath, fragSource)) {
        SHADER_ERR(("Failed to load fragment shader: " + fragPath).c_str());
        return false;
    }

    SHADER_LOG("Shader files loaded, compiling...");
    return loadFromSource(vertSource, fragSource);
}

bool Shader::loadFromSource(const std::string& vertSource, const std::string& fragSource) {
    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSource);
    if (!vertShader) return false;

    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource);
    if (!fragShader) {
        glDeleteShader(vertShader);
        return false;
    }

    bool success = linkProgram(vertShader, fragShader);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return success;
}

void Shader::use() const {
    glUseProgram(m_program);
}

void Shader::setMat4(const char* name, const float* data) const {
    GLint loc = glGetUniformLocation(m_program, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, data);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    GLint loc = glGetUniformLocation(m_program, name);
    glUniform3f(loc, x, y, z);
}

void Shader::setFloat(const char* name, float value) const {
    GLint loc = glGetUniformLocation(m_program, name);
    glUniform1f(loc, value);
}

void Shader::setInt(const char* name, int value) const {
    GLint loc = glGetUniformLocation(m_program, name);
    glUniform1i(loc, value);
}

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::linkProgram(GLuint vertShader, GLuint fragShader) {
    m_program = glCreateProgram();
    glAttachShader(m_program, vertShader);
    glAttachShader(m_program, fragShader);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    return true;
}

} // namespace polarclock

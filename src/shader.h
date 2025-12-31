#pragma once

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <string>

namespace polarclock {

class Shader {
public:
    Shader() : m_program(0) {}
    ~Shader();

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    bool loadFromSource(const std::string& vertSource, const std::string& fragSource);

    void use() const;
    GLuint getProgram() const { return m_program; }

    // Uniform setters
    void setMat4(const char* name, const float* data) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setFloat(const char* name, float value) const;
    void setInt(const char* name, int value) const;

private:
    GLuint m_program;

    GLuint compileShader(GLenum type, const std::string& source);
    bool linkProgram(GLuint vertShader, GLuint fragShader);
};

} // namespace polarclock

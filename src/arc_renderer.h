#pragma once

#include "shader.h"
#include "polar_clock.h"
#include "pcmath.h"
#include <vector>

namespace polarclock {

class ArcRenderer {
public:
    ArcRenderer();
    ~ArcRenderer();

    bool init();
    void render(const PolarClock& clock, const math::Mat4& projection);

    // Render a single arc with explicit parameters
    void renderArc(float innerRadius, float outerRadius, float value,
                   const math::Vec3& color, const math::Mat4& projection);

private:
    void generateArcGeometry(float innerRadius, float outerRadius, float endAngle,
                             std::vector<float>& vertices);
    void generateQuarterCircle(float centerX, float centerY, float radius,
                               float startAngle, float endAngle,
                               std::vector<float>& vertices);

    Shader m_shader;
    GLuint m_vao;
    GLuint m_vbo;

    static constexpr int SEGMENTS = 128;  // Segments per full circle
};

} // namespace polarclock

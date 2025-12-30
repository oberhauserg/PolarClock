#include "arc_renderer.h"
#include <vector>
#include <cmath>

namespace polarclock {

ArcRenderer::ArcRenderer()
    : m_vao(0)
    , m_vbo(0)
{
}

ArcRenderer::~ArcRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool ArcRenderer::init() {
    if (!m_shader.loadFromFiles("/shaders/arc.vert", "/shaders/arc.frag")) {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Position attribute (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

void ArcRenderer::generateQuarterCircle(float centerX, float centerY, float radius,
                                         float startAngle, float endAngle,
                                         std::vector<float>& vertices) {
    const int cornerSegments = 6;

    for (int i = 0; i < cornerSegments; ++i) {
        float t0 = static_cast<float>(i) / cornerSegments;
        float t1 = static_cast<float>(i + 1) / cornerSegments;

        float angle0 = startAngle + t0 * (endAngle - startAngle);
        float angle1 = startAngle + t1 * (endAngle - startAngle);

        // Triangle from center to two edge points
        vertices.push_back(centerX);
        vertices.push_back(centerY);

        vertices.push_back(centerX + radius * std::cos(angle0));
        vertices.push_back(centerY + radius * std::sin(angle0));

        vertices.push_back(centerX + radius * std::cos(angle1));
        vertices.push_back(centerY + radius * std::sin(angle1));
    }
}

void ArcRenderer::generateArcGeometry(float innerRadius, float outerRadius, float endAngle,
                                       std::vector<float>& vertices) {
    vertices.clear();

    if (endAngle <= 0.001f) return;

    float ringThickness = outerRadius - innerRadius;
    float cr = ringThickness * 0.2f;  // Small corner radius (20% of thickness)

    float arcStart = math::PI / 2.0f;  // 12 o'clock
    float sweep = endAngle * math::TAU;
    float arcEnd = arcStart - sweep;

    int numSegments = static_cast<int>(SEGMENTS * endAngle) + 1;
    numSegments = std::max(numSegments, 3);

    // we need to offset the corner into the curve. To do this, we'll take the tangent to the end of the curve,
    // and offset the size of the corner. Then figure out what angle that makes on the curve.
    float outer_corner_angle = std::atan(cr/outerRadius);
    float inner_corner_angle = std::atan(cr/innerRadius);

    // Generate the main arc body with triangular cutouts at corners

    float arc_endcap_radian_size = inner_corner_angle;

    float arc_main_start = arcStart - arc_endcap_radian_size;
    float last_angle = arc_main_start;
    //float arc_main_sweep = (endAngle - (arc_endcap_angular_size*2)) * math::TAU;
    float arc_main_sweep = (sweep - arc_endcap_radian_size * 2);

    for (int i = 0; i < numSegments; ++i) {
        float t0 = static_cast<float>(i) / numSegments;
        float t1 = static_cast<float>(i + 1) / numSegments;

        // Determine radii for this segment (inset at start/end for corner cutouts)
        float inner0 = innerRadius;
        float outer0 = outerRadius;
        float inner1 = innerRadius;
        float outer1 = outerRadius;

        float a0 = last_angle;
        float a1 = arc_main_start - t1 * arc_main_sweep;

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        // Generate quad as two triangles
        vertices.push_back(inner0 * c0);
        vertices.push_back(inner0 * s0);
        vertices.push_back(outer0 * c0);
        vertices.push_back(outer0 * s0);
        vertices.push_back(inner1 * c1);
        vertices.push_back(inner1 * s1);

        vertices.push_back(inner1 * c1);
        vertices.push_back(inner1 * s1);
        vertices.push_back(outer0 * c0);
        vertices.push_back(outer0 * s0);
        vertices.push_back(outer1 * c1);
        vertices.push_back(outer1 * s1);

        last_angle = a1;
    }

    /*
     * After the main arc, render the curved endcaps.
     *
     * To do this, we're rendering quarter-circles on the corners.
     * Which means we need the equation for a circle to determine the distance of the edge from the center of the arc.
     * 
     * Namely, we need to determine the outer edge distance for any x position along the arc.
     * 
     * The equation for a circle is (x−a)^2 + (y−b)^2 = r^2 where (a,b) is the center of the circle.
     * 
     * For our case, we only need to know where we are laterally along the circle, so we can determine where the (x,y) point is on the circle,
     * then finally we need to translate that to the edge position.
     * 
     * To make this capable of different edge radii, let's assume a radius of r.
     * 
     * Then, we only need to know distance from the center, we need to solve for y, given an x. The (a,b) can be assumed to be (0,0).
     * 
     * Then we have (x)^2 + (y)^2 = r^2. To solve for y, we have y = sqrt(x^2 + r^2).
     * 
     * To translate this value to the distance from the arc center, we simply need arc_thickness/2 - r + sqrt(x^2 + r^2).
     */
    last_angle = arcStart;
    float start_endcap_end = arcStart + arc_endcap_radian_size;
    size_t num_endcap_segments = 20;
    for(int i = 0; i < num_endcap_segments; i++)
    {
        float t0 = static_cast<float>(i) / num_endcap_segments;
        float t1 = static_cast<float>(i + 1) / num_endcap_segments;

        float a0 = arcStart - t0 * arc_endcap_radian_size;
        float a1 = arcStart - t1 * arc_endcap_radian_size;

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);


        // Determine radii for this segment (inset at start/end for corner cutouts)
        
        
        // For both angles, determine the arc thickness.
        // We need the arc-length from the end of the arc.
        float arc_thickness = outerRadius - innerRadius;
        float arc_midpoint_distance = (outerRadius + innerRadius)/2;
        
        float a0_outer_arc_distance = outerRadius * std::abs(arcStart - a0) - cr;
        float a1_outer_arc_distance = outerRadius * std::abs(arcStart - a1) - cr;

        float a0_inner_arc_distance = innerRadius * std::abs(arcStart - a0) - cr;
        float a1_inner_arc_distance = innerRadius * std::abs(arcStart - a1) - cr;


        // float a0_arc_distance = arc_midpoint_distance * std::abs(arcStart - a0) - cr;
        // float a1_arc_distance = arc_midpoint_distance * std::abs(arcStart - a1) - cr;
        
        if(a0_inner_arc_distance < cr && a1_inner_arc_distance < cr &&
           a0_outer_arc_distance < cr && a1_outer_arc_distance < cr)
           {
            float outer0 = outerRadius - cr + sqrt(cr * cr - a0_outer_arc_distance * a0_inner_arc_distance);
            float outer1 = outerRadius - cr + sqrt(cr * cr - a1_outer_arc_distance * a1_inner_arc_distance);
            float inner0 = innerRadius + cr - sqrt(cr * cr - a0_inner_arc_distance * a0_inner_arc_distance);
            float inner1 = innerRadius + cr - sqrt(cr * cr - a1_inner_arc_distance * a1_inner_arc_distance);

            // Generate quad as two triangles
            vertices.push_back(inner0 * c0);
            vertices.push_back(inner0 * s0);
            vertices.push_back(outer0 * c0);
            vertices.push_back(outer0 * s0);
            vertices.push_back(inner1 * c1);
            vertices.push_back(inner1 * s1);

            vertices.push_back(inner1 * c1);
            vertices.push_back(inner1 * s1);
            vertices.push_back(outer0 * c0);
            vertices.push_back(outer0 * s0);
            vertices.push_back(outer1 * c1);
            vertices.push_back(outer1 * s1);



        }

    }


    float end_endcap_start = arc_main_start - arc_main_sweep;
    for(int i = 0; i < num_endcap_segments; i++)
    {
        float t0 = static_cast<float>(i) / num_endcap_segments;
        float t1 = static_cast<float>(i + 1) / num_endcap_segments;

        float a0 = end_endcap_start - t0 * arc_endcap_radian_size;
        float a1 = end_endcap_start - t1 * arc_endcap_radian_size;

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);


        // Determine radii for this segment (inset at start/end for corner cutouts)
        
        
        // For both angles, determine the arc thickness.
        // We need the arc-length from the end of the arc.
        float arc_thickness = outerRadius - innerRadius;
        float arc_midpoint_distance = (outerRadius + innerRadius)/2;
        
        float a0_outer_arc_distance = outerRadius * std::abs(arcEnd - a0) - cr;
        float a1_outer_arc_distance = outerRadius * std::abs(arcEnd - a1) - cr;

        float a0_inner_arc_distance = innerRadius * std::abs(arcEnd - a0) - cr;
        float a1_inner_arc_distance = innerRadius * std::abs(arcEnd - a1) - cr;


        // float a0_arc_distance = arc_midpoint_distance * std::abs(arcStart - a0) - cr;
        // float a1_arc_distance = arc_midpoint_distance * std::abs(arcStart - a1) - cr;
        
        if(a0_inner_arc_distance < cr && a1_inner_arc_distance < cr &&
           a0_outer_arc_distance < cr && a1_outer_arc_distance < cr)
           {
            float outer0 = outerRadius - cr + sqrt(cr * cr - a0_outer_arc_distance * a0_inner_arc_distance);
            float outer1 = outerRadius - cr + sqrt(cr * cr - a1_outer_arc_distance * a1_inner_arc_distance);
            float inner0 = innerRadius + cr - sqrt(cr * cr - a0_inner_arc_distance * a0_inner_arc_distance);
            float inner1 = innerRadius + cr - sqrt(cr * cr - a1_inner_arc_distance * a1_inner_arc_distance);

            // Generate quad as two triangles
            vertices.push_back(inner0 * c0);
            vertices.push_back(inner0 * s0);
            vertices.push_back(outer0 * c0);
            vertices.push_back(outer0 * s0);
            vertices.push_back(inner1 * c1);
            vertices.push_back(inner1 * s1);

            vertices.push_back(inner1 * c1);
            vertices.push_back(inner1 * s1);
            vertices.push_back(outer0 * c0);
            vertices.push_back(outer0 * s0);
            vertices.push_back(outer1 * c1);
            vertices.push_back(outer1 * s1);



        }

    }

}

void ArcRenderer::render(const PolarClock& clock, const math::Mat4& projection) {
    m_shader.use();
    m_shader.setMat4("u_projection", projection.data());

    glBindVertexArray(m_vao);

    for (const auto& ring : clock.getRings()) {
        if (ring.currentValue <= 0.001f) continue;

        std::vector<float> vertices;
        generateArcGeometry(ring.innerRadius, ring.outerRadius, ring.currentValue, vertices);

        if (vertices.empty()) continue;

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        m_shader.setVec3("u_colorBase", ring.colors.base.x, ring.colors.base.y, ring.colors.base.z);

        // Draw as triangles (2 floats per vertex)
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 2));
    }

    glBindVertexArray(0);
}

void ArcRenderer::renderArc(float innerRadius, float outerRadius, float value,
                             const math::Vec3& color, const math::Mat4& projection) {
    if (value <= 0.001f) return;

    m_shader.use();
    m_shader.setMat4("u_projection", projection.data());
    m_shader.setVec3("u_colorBase", color.x, color.y, color.z);

    glBindVertexArray(m_vao);

    std::vector<float> vertices;
    generateArcGeometry(innerRadius, outerRadius, value, vertices);

    if (!vertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 2));
    }

    glBindVertexArray(0);
}

} // namespace polarclock

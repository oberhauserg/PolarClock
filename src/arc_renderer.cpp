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

/**
 * @brief Initialize OpenGL resources for arc rendering.
 *
 * Loads the arc shader and creates VAO/VBO for dynamic geometry.
 * The vertex format is simple: vec2 position only.
 *
 * @return true if initialization succeeded, false otherwise.
 */
bool ArcRenderer::init() {
    if (!m_shader.loadFromFiles("/shaders/arc.vert", "/shaders/arc.frag")) {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

/**
 * @brief Push a quad (two triangles) to the vertex buffer.
 *
 * Creates a quadrilateral segment of an arc by generating two triangles.
 * Each edge of the quad is defined by an inner and outer radius at a given angle.
 *
 * @param vertices Output vertex buffer to append to.
 * @param inner0   Inner radius at the first angle.
 * @param outer0   Outer radius at the first angle.
 * @param c0       Cosine of the first angle.
 * @param s0       Sine of the first angle.
 * @param inner1   Inner radius at the second angle.
 * @param outer1   Outer radius at the second angle.
 * @param c1       Cosine of the second angle.
 * @param s1       Sine of the second angle.
 */
static void pushQuad(std::vector<float>& vertices,
                     double inner0, double outer0, double c0, double s0,
                     double inner1, double outer1, double c1, double s1) {
    // Triangle 1: inner0 -> outer0 -> inner1
    vertices.push_back(inner0 * c0);
    vertices.push_back(inner0 * s0);
    vertices.push_back(outer0 * c0);
    vertices.push_back(outer0 * s0);
    vertices.push_back(inner1 * c1);
    vertices.push_back(inner1 * s1);

    // Triangle 2: inner1 -> outer0 -> outer1
    vertices.push_back(inner1 * c1);
    vertices.push_back(inner1 * s1);
    vertices.push_back(outer0 * c0);
    vertices.push_back(outer0 * s0);
    vertices.push_back(outer1 * c1);
    vertices.push_back(outer1 * s1);
}

/**
 * @brief Generate rounded endcap geometry for an arc.
 *
 * Creates the rounded corner effect at the start or end of an arc using the
 * circle equation to calculate the inner/outer edge positions at each angle.
 *
 * The algorithm works by:
 * 1. Sweeping through the endcap angular region in small segments.
 * 2. For each segment, calculating the arc-length distance from the arc's edge.
 * 3. Using the circle equation (x² + y² = r²) to determine how far the inner/outer
 *    edges should be inset to create the rounded corner effect.
 *
 * The circle equation is solved for y given x:
 *   y = sqrt(r² - x²)
 *
 * This gives the offset from the corner center, which is then applied to create
 * the rounded edge profile.
 *
 * @param vertices       Output vertex buffer to append to.
 * @param innerRadius    Inner radius of the arc ring.
 * @param outerRadius    Outer radius of the arc ring.
 * @param cr             Corner radius for the rounded effect.
 * @param endcapStart    Starting angle of the endcap region (radians).
 * @param endcapSize     Angular size of the endcap region (radians).
 * @param referenceAngle The angle of the arc's edge (used to calculate distances).
 */
static void generateEndcap(std::vector<float>& vertices,
                           double innerRadius, double outerRadius, double cr,
                           double endcapStart, double endcapSize, double referenceAngle) {
    const int numSegments = 12;

    for (int i = 0; i < numSegments; ++i) {
        double t0 = static_cast<float>(i) / numSegments;
        double t1 = static_cast<float>(i + 1) / numSegments;

        double a0 = endcapStart - t0 * endcapSize;
        double a1 = endcapStart - t1 * endcapSize;

        double c0 = std::cos(a0), s0 = std::sin(a0);
        double c1 = std::cos(a1), s1 = std::sin(a1);

        // Calculate arc-length distance from the reference angle (arc edge)
        // This represents the "x" value in our circle equation
        double a0OuterDist = outerRadius * std::abs(referenceAngle - a0) - cr;
        double a1OuterDist = outerRadius * std::abs(referenceAngle - a1) - cr;
        double a0InnerDist = innerRadius * std::abs(referenceAngle - a0) - cr;
        double a1InnerDist = innerRadius * std::abs(referenceAngle - a1) - cr;

        // Only render if within the corner radius region
        if (a0InnerDist < cr && a1InnerDist < cr &&
            a0OuterDist < cr && a1OuterDist < cr) {
            // Apply circle equation: y = sqrt(r² - x²) to find edge offset
            // For outer edge: radius decreases (inset toward center)
            // For inner edge: radius increases (inset away from center)
            double outer0 = outerRadius - cr + std::sqrt(cr * cr - a0OuterDist * a0InnerDist);
            double outer1 = outerRadius - cr + std::sqrt(cr * cr - a1OuterDist * a1InnerDist);
            double inner0 = innerRadius + cr - std::sqrt(cr * cr - a0InnerDist * a0InnerDist);
            double inner1 = innerRadius + cr - std::sqrt(cr * cr - a1InnerDist * a1InnerDist);

            pushQuad(vertices, inner0, outer0, c0, s0, inner1, outer1, c1, s1);
        }
    }
}

/**
 * @brief Generate complete arc geometry including rounded corners.
 *
 * Creates the vertex data for an arc segment with rounded corners at all four
 * corners. The arc starts at 12 o'clock and sweeps clockwise.
 *
 * The geometry is composed of three parts:
 * 1. Start endcap - rounded corners at the arc's starting edge
 * 2. Main body - the rectangular portion of the arc
 * 3. End endcap - rounded corners at the arc's ending edge
 *
 * The corner radius is calculated as 20% of the ring thickness, creating
 * a subtle rounded rectangle appearance.
 *
 * @param innerRadius Inner radius of the arc ring.
 * @param outerRadius Outer radius of the arc ring.
 * @param endAngle    Arc sweep as a fraction of a full circle (0.0 to 1.0).
 * @param vertices    Output buffer to fill with vertex data (cleared first).
 */
void ArcRenderer::generateArcGeometry(double innerRadius, double outerRadius, double endAngle,
                                       std::vector<float>& vertices) {
    vertices.clear();

    if (endAngle <= 0.001f) return;

    double ringThickness = outerRadius - innerRadius;
    double cr = ringThickness * 0.1;  // Corner radius (20% of thickness)

    double arcStart = math::PI / 2.0;  // 12 o'clock
    double sweep = endAngle * math::TAU;
    double arcEnd = arcStart - sweep;

    // Calculate angular size of endcaps using tangent offset
    double endcapAngularSize = std::atan(cr / innerRadius);

    // Main arc body runs between the two endcap regions
    double mainStart = arcStart - endcapAngularSize;
    double mainSweep = sweep - endcapAngularSize * 2.0;

    // Generate main arc body
    int numSegments = static_cast<int>(SEGMENTS * endAngle) + 1;
    numSegments = std::max(numSegments, 3);

    double lastAngle = mainStart;
    for (int i = 0; i < numSegments; ++i) {
        double t1 = static_cast<float>(i + 1) / numSegments;

        double a0 = lastAngle;
        double a1 = mainStart - t1 * mainSweep;

        double c0 = std::cos(a0), s0 = std::sin(a0);
        double c1 = std::cos(a1), s1 = std::sin(a1);

        pushQuad(vertices, innerRadius, outerRadius, c0, s0,
                          innerRadius, outerRadius, c1, s1);

        lastAngle = a1;
    }

    // Generate start endcap (rounded corners at arc start)
    generateEndcap(vertices, innerRadius, outerRadius, cr,
                   arcStart, endcapAngularSize, arcStart);

    // Generate end endcap (rounded corners at arc end)
    double endEndcapStart = mainStart - mainSweep;
    generateEndcap(vertices, innerRadius, outerRadius, cr,
                   endEndcapStart, endcapAngularSize, arcEnd);
}

/**
 * @brief Render all arcs for a polar clock.
 *
 * Iterates through all rings in the clock and renders each one as an arc
 * with the appropriate color. Geometry is regenerated each frame to support
 * smooth animation of the arc sweep values.
 *
 * @param clock      The PolarClock containing ring data to render.
 * @param projection The projection matrix for coordinate transformation.
 */
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

/**
 * @brief Render a single arc with explicit parameters.
 *
 * Convenience method for rendering an individual arc without needing a
 * PolarClock instance. Useful for rendering arcs with custom colors or
 * values that differ from the clock's current state.
 *
 * @param innerRadius Inner radius of the arc ring.
 * @param outerRadius Outer radius of the arc ring.
 * @param value       Arc sweep as a fraction of a full circle (0.0 to 1.0).
 * @param color       RGB color for the arc.
 * @param projection  The projection matrix for coordinate transformation.
 */
void ArcRenderer::renderArc(double innerRadius, double outerRadius, double value,
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

#include "renderer.h"
#include <cmath>

namespace polarclock {

Renderer::Renderer()
    : m_width(800)
    , m_height(800)
    , m_scale(1.0f)
{
    m_theme = createDefaultTheme();
}

bool Renderer::init(int width, int height) {
    m_width = width;
    m_height = height;

    if (!m_arcRenderer.init()) {
        return false;
    }

    if (!m_textRenderer.init("/assets/Roboto-Regular.ttf", 24.0f)) {
        return false;
    }

    resize(width, height);
    return true;
}

void Renderer::resize(int width, int height) {
    m_width = width;
    m_height = height;

    // Use the smaller dimension to maintain aspect ratio
    int minDim = std::min(width, height);
    m_scale = minDim / 2.0f;

    // Create orthographic projection centered at origin, with -1 to 1 range
    float aspect = static_cast<float>(width) / height;
    if (aspect >= 1.0f) {
        m_projection = math::Mat4::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        m_projection = math::Mat4::ortho(-1.0f, 1.0f, -1.0f/aspect, 1.0f/aspect, -1.0f, 1.0f);
    }

    glViewport(0, 0, width, height);
}

void Renderer::setTheme(const Theme& theme) {
    m_theme = theme;
}

float Renderer::calculateMinArcValue(const Ring& ring) {
    // Calculate text properties
    float ringThickness = ring.outerRadius - ring.innerRadius;
    float textScale = ringThickness * 0.020f;
    float radius = (ring.innerRadius + ring.outerRadius) / 2.0f;

    // Calculate how much angular space the text needs
    float textWidth = m_textRenderer.getTextWidth(ring.valueText, textScale);
    float textAngularSpan = textWidth / radius;

    // Add padding on both sides
    float padding = ringThickness * 0.5f / radius;
    float minSweepNeeded = textAngularSpan + padding * 2.0f;

    // Convert to 0-1 range (sweep / TAU)
    return minSweepNeeded / math::TAU;
}

void Renderer::render(const PolarClock& clock) {
    // Clear with background color
    glClearColor(m_theme.background.x, m_theme.background.y, m_theme.background.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render arcs and labels with minimum arc sizes enforced
    for (const auto& ring : clock.getRings()) {
        float minValue = calculateMinArcValue(ring);
        float effectiveValue = std::max(ring.currentValue, minValue);

        // Render arc with effective value
        m_arcRenderer.renderArc(
            ring.innerRadius, ring.outerRadius,
            effectiveValue,
            ring.colors.base,
            m_projection
        );

        // Render label
        renderLabel(ring, effectiveValue);
    }

    glDisable(GL_BLEND);
}

void Renderer::renderLabel(const Ring& ring, float effectiveValue) {
    // Calculate text properties
    float ringThickness = ring.outerRadius - ring.innerRadius;
    float textScale = ringThickness * 0.020f;

    // Position at center of ring thickness
    float radius = (ring.innerRadius + ring.outerRadius) / 2.0f;

    std::string label = ring.valueText;

    // Calculate how much angular space the text needs
    float textWidth = m_textRenderer.getTextWidth(label, textScale);
    float textAngularSpan = textWidth / radius;

    // Calculate arc angles using effective value
    float startAngle = math::PI / 2.0f;  // Top (12 o'clock)
    float sweepAngle = effectiveValue * math::TAU;
    float arcEndAngle = startAngle - sweepAngle;

    // Center the text near the end of the arc
    float padding = ringThickness * 0.5f / radius;
    float textCenterAngle = arcEndAngle + textAngularSpan / 2.0f + padding;

    // Use dark color for contrast against bright arc
    math::Vec3 textColor(0.05f, 0.05f, 0.05f);

    m_textRenderer.renderTextOnArc(
        label,
        radius,
        textCenterAngle,
        textScale,
        textColor,
        m_projection,
        true,  // clockwise (text follows arc direction)
        1.0f
    );
}

} // namespace polarclock

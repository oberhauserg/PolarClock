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

    if (!m_textRenderer.init("assets/RobotoMono-Bold.ttf", 72.0f)) {
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

float Renderer::calculateMinArcValue(const Ring& ring, float scale) {
    // Calculate text properties
    float ringThickness = ring.outerRadius * scale - ring.innerRadius * scale;
    float textScale = ringThickness * 0.005f * scale;
    float radius = ring.outerRadius * scale - m_textRenderer.getTextHeight(ring.valueText, textScale);

    // Calculate how much angular space the text needs
    float textWidth = m_textRenderer.getTextWidth(ring.valueText, textScale);
    float textAngularSpan = textWidth / radius;

    // Add padding on both sides
    float padding = ringThickness * 0.1f / radius;
    float minSweepNeeded = textAngularSpan + padding * 2.0f;

    // Convert to 0-1 range (sweep / TAU)
    return minSweepNeeded / math::TAU;
}

void Renderer::render(const PolarClock& clock) {
    // Clear with background color from clock's theme
    const auto& bg = clock.getTheme().background;
    glClearColor(bg.x, bg.y, bg.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Create a scale factor to make everything nicely fit to the screen.
    float ring_scale = .9 / clock.getMaxRadius();

    // Enable blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render arcs and labels with minimum arc sizes enforced
    for (const auto& ring : clock.getRings()) {
        float minValue = calculateMinArcValue(ring, ring_scale);
        float effectiveValue = std::max(ring.currentValue, minValue);

        // Interpolate color from bright (at 0) to base (at 1)
        // This makes rings reset to bright/merry colors on NYE
        float t = ring.currentValue;
        math::Vec3 arcColor(
            ring.colors.bright.x + (ring.colors.base.x - ring.colors.bright.x) * t,
            ring.colors.bright.y + (ring.colors.base.y - ring.colors.bright.y) * t,
            ring.colors.bright.z + (ring.colors.base.z - ring.colors.bright.z) * t
        );

        // Render arc with effective value and interpolated color
        m_arcRenderer.renderArc(
            ring.innerRadius * ring_scale, ring.outerRadius * ring_scale,
            effectiveValue,
            arcColor,
            m_projection
        );

        // Render label
        renderLabel(ring, effectiveValue, ring_scale);
    }

    glDisable(GL_BLEND);
}

void Renderer::renderLabel(const Ring& ring, float effectiveValue, float scale) {
    // Calculate text properties
    float ringThickness = ring.outerRadius * scale - ring.innerRadius * scale;
    float textScale = ringThickness * 0.005f * scale;

    std::string label = ring.valueText;

    // Position at center of ring thickness
    //float radius = (ring.innerRadius + ring.outerRadius) / 2.0f;
    float radius = ring.outerRadius * scale - m_textRenderer.getTextHeight(label, textScale);

    // Calculate how much angular space the text needs
    float textWidth = m_textRenderer.getTextWidth(label, textScale);
    float textAngularSpan = textWidth / radius;

    // Calculate arc angles using effective value
    float startAngle = math::PI / 2.0f;  // Top (12 o'clock)
    float sweepAngle = effectiveValue * math::TAU;
    float arcEndAngle = startAngle - sweepAngle;

    // Center the text near the end of the arc
    float padding = ringThickness * 0.1f / radius;
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

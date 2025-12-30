#pragma once

#include "arc_renderer.h"
#include "text_renderer.h"
#include "polar_clock.h"
#include "theme.h"
#include "pcmath.h"

namespace polarclock {

class Renderer {
public:
    Renderer();
    ~Renderer() = default;

    bool init(int width, int height);
    void resize(int width, int height);
    void render(const PolarClock& clock);
    void setTheme(const Theme& theme);

private:
    void renderLabel(const Ring& ring, float effectiveValue, float scale);
    float calculateMinArcValue(const Ring& ring, float scale);

    ArcRenderer m_arcRenderer;
    TextRenderer m_textRenderer;
    Theme m_theme;

    math::Mat4 m_projection;
    int m_width;
    int m_height;
    float m_scale;
};

} // namespace polarclock

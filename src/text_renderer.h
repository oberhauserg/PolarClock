#pragma once

#include "shader.h"
#include "pcmath.h"
#include <string>
#include <unordered_map>

namespace polarclock {

struct GlyphInfo {
    float x0, y0, x1, y1;  // Texture coordinates
    float xoff, yoff;       // Offset from cursor
    float xadvance;         // Advance to next character
    float width, height;    // Glyph dimensions
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool init(const std::string& fontPath, float fontSize);
    void renderText(const std::string& text, float x, float y, float scale,
                    const math::Vec3& color, const math::Mat4& projection,
                    float rotation = 0.0f, float alpha = 1.0f, bool centered = false);

    // Render text curved along an arc
    // centerAngle: angle where text should be centered (radians)
    // radius: distance from origin to place text
    // clockwise: if true, text curves clockwise from centerAngle
    void renderTextOnArc(const std::string& text, float radius, float centerAngle,
                         float scale, const math::Vec3& color, const math::Mat4& projection,
                         bool clockwise = true, float alpha = 1.0f);

    float getTextWidth(const std::string& text, float scale) const;
    float getTextHeight(const std::string& text, float scale) const;

private:
    Shader m_shader;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_fontTexture;

    std::unordered_map<char, GlyphInfo> m_glyphs;
    float m_fontSize;
    int m_atlasWidth;
    int m_atlasHeight;
};

} // namespace polarclock

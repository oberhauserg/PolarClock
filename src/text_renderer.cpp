#include "text_renderer.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace polarclock {

TextRenderer::TextRenderer()
    : m_vao(0)
    , m_vbo(0)
    , m_fontTexture(0)
    , m_fontSize(32.0f)
    , m_atlasWidth(512)
    , m_atlasHeight(512)
{
}

TextRenderer::~TextRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_fontTexture) glDeleteTextures(1, &m_fontTexture);
}

bool TextRenderer::init(const std::string& fontPath, float fontSize) {
    m_fontSize = fontSize;

    // Load font file
    std::ifstream fontFile(fontPath, std::ios::binary | std::ios::ate);
    if (!fontFile.is_open()) {
        std::cerr << "Failed to open font file: " << fontPath << std::endl;
        return false;
    }

    size_t fileSize = fontFile.tellg();
    fontFile.seekg(0);
    std::vector<unsigned char> fontBuffer(fileSize);
    fontFile.read(reinterpret_cast<char*>(fontBuffer.data()), fileSize);

    // Initialize stb_truetype
    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)) {
        std::cerr << "Failed to initialize font" << std::endl;
        return false;
    }

    // Create font atlas
    std::vector<unsigned char> atlasData(m_atlasWidth * m_atlasHeight, 0);
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

    int x = 2, y = 2;
    int maxRowHeight = 0;

    // Bake ASCII characters 32-126
    for (char c = 32; c < 127; ++c) {
        int width, height, xoff, yoff;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(
            &fontInfo, 0, scale, c, &width, &height, &xoff, &yoff);

        if (x + width + 2 >= m_atlasWidth) {
            x = 2;
            y += maxRowHeight + 2;
            maxRowHeight = 0;
        }

        if (y + height + 2 >= m_atlasHeight) {
            std::cerr << "Font atlas too small!" << std::endl;
            stbtt_FreeBitmap(bitmap, nullptr);
            break;
        }

        // Copy glyph to atlas
        for (int row = 0; row < height; ++row) {
            std::memcpy(&atlasData[(y + row) * m_atlasWidth + x],
                       &bitmap[row * width], width);
        }

        // Store glyph info
        GlyphInfo glyph;
        glyph.x0 = static_cast<float>(x) / m_atlasWidth;
        glyph.y0 = static_cast<float>(y) / m_atlasHeight;
        glyph.x1 = static_cast<float>(x + width) / m_atlasWidth;
        glyph.y1 = static_cast<float>(y + height) / m_atlasHeight;
        glyph.xoff = static_cast<float>(xoff);
        glyph.yoff = static_cast<float>(yoff);
        glyph.width = static_cast<float>(width);
        glyph.height = static_cast<float>(height);

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&fontInfo, c, &advanceWidth, &leftSideBearing);
        glyph.xadvance = advanceWidth * scale;

        m_glyphs[c] = glyph;

        x += width + 2;
        maxRowHeight = std::max(maxRowHeight, height);

        stbtt_FreeBitmap(bitmap, nullptr);
    }

    // Create OpenGL texture
    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_atlasWidth, m_atlasHeight,
                 0, GL_RED, GL_UNSIGNED_BYTE, atlasData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Load shader
    if (!m_shader.loadFromFiles("/shaders/text.vert", "/shaders/text.frag")) {
        return false;
    }

    // Create VAO/VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    // Position (vec2) + TexCoord (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale,
                               const math::Vec3& color, const math::Mat4& projection,
                               float rotation, float alpha, bool centered) {
    m_shader.use();
    m_shader.setMat4("u_projection", projection.data());
    m_shader.setVec3("u_textColor", color.x, color.y, color.z);
    m_shader.setFloat("u_alpha", alpha);
    m_shader.setInt("u_fontTexture", 0);

    // Create model matrix for rotation around the text position
    math::Mat4 model = math::Mat4::translate(x, y, 0.0f) *
                       math::Mat4::rotate(rotation) *
                       math::Mat4::scale(scale, scale, 1.0f);
    m_shader.setMat4("u_model", model.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glBindVertexArray(m_vao);

    // Calculate starting cursor position (centered if requested)
    float cursorX = 0;
    float cursorY = 0;

    if (centered) {
        // Calculate unscaled text width for centering (scale is in model matrix)
        float textWidth = getTextWidth(text, 1.0f);
        cursorX = -textWidth / 2.0f;
        // Center vertically (approximate: baseline sits at fontSize/4 above center)
        cursorY = -m_fontSize / 4.0f;
    }

    for (char c : text) {
        auto it = m_glyphs.find(c);
        if (it == m_glyphs.end()) continue;

        const GlyphInfo& g = it->second;

        float xpos = cursorX + g.xoff;
        // Flip Y: stb_truetype uses Y-down, OpenGL uses Y-up
        // yoff is negative for glyphs above baseline, so negate it
        float ypos = cursorY - g.yoff - g.height;

        float w = g.width;
        float h = g.height;

        // 6 vertices for a quad (2 triangles)
        // ypos is bottom, ypos+h is top (OpenGL Y-up)
        // Texture y0 is top of glyph, y1 is bottom
        float vertices[] = {
            xpos,     ypos,     g.x0, g.y1,  // bottom-left
            xpos,     ypos + h, g.x0, g.y0,  // top-left
            xpos + w, ypos + h, g.x1, g.y0,  // top-right

            xpos,     ypos,     g.x0, g.y1,  // bottom-left
            xpos + w, ypos + h, g.x1, g.y0,  // top-right
            xpos + w, ypos,     g.x1, g.y1   // bottom-right
        };

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        cursorX += g.xadvance;
    }

    glBindVertexArray(0);
}

void TextRenderer::renderTextOnArc(const std::string& text, float radius, float centerAngle,
                                    float scale, const math::Vec3& color, const math::Mat4& projection,
                                    bool clockwise, float alpha) {
    if (text.empty()) return;

    m_shader.use();
    m_shader.setMat4("u_projection", projection.data());
    m_shader.setVec3("u_textColor", color.x, color.y, color.z);
    m_shader.setFloat("u_alpha", alpha);
    m_shader.setInt("u_fontTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glBindVertexArray(m_vao);

    // Calculate total text width (unscaled)
    float totalWidth = getTextWidth(text, 1.0f);

    // Convert width to angular span on the arc
    // Arc length = radius * angle, so angle = arc_length / radius
    float angularSpan = (totalWidth * scale) / radius;

    // Direction multiplier (clockwise = negative angle change)
    float dir = clockwise ? -1.0f : 1.0f;

    // Start angle: offset by half the span to center the text
    float currentAngle = centerAngle - dir * angularSpan / 2.0f;

    for (char c : text) {
        auto it = m_glyphs.find(c);
        if (it == m_glyphs.end()) continue;

        const GlyphInfo& g = it->second;

        // Character width in angular terms
        float charAngularWidth = (g.xadvance * scale) / radius;

        // Position at center of this character
        float charAngle = currentAngle + dir * charAngularWidth / 2.0f;

        // Position on the arc
        float x = radius * std::cos(charAngle);
        float y = radius * std::sin(charAngle);

        // Rotation: tangent to arc (perpendicular to radius)
        // For clockwise text, tangent points in direction of decreasing angle
        float rotation = charAngle - math::PI / 2.0f;
        if (!clockwise) {
            rotation = charAngle + math::PI / 2.0f;
        }

        // Create model matrix: translate to position, rotate, scale
        math::Mat4 model = math::Mat4::translate(x, y, 0.0f) *
                           math::Mat4::rotate(rotation) *
                           math::Mat4::scale(scale, scale, 1.0f);
        m_shader.setMat4("u_model", model.data());

        // Center the glyph at origin (so rotation is around center)
        float halfW = g.width / 2.0f;
        float halfH = g.height / 2.0f;
        float yOffset = -m_fontSize / 4.0f;  // Baseline adjustment

        float xpos = -halfW + g.xoff;
        float ypos = yOffset - g.yoff - g.height;

        float w = g.width;
        float h = g.height;

        // 6 vertices for a quad
        float vertices[] = {
            xpos,     ypos,     g.x0, g.y1,
            xpos,     ypos + h, g.x0, g.y0,
            xpos + w, ypos + h, g.x1, g.y0,

            xpos,     ypos,     g.x0, g.y1,
            xpos + w, ypos + h, g.x1, g.y0,
            xpos + w, ypos,     g.x1, g.y1
        };

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance to next character position
        currentAngle += dir * charAngularWidth;
    }

    glBindVertexArray(0);
}

float TextRenderer::getTextWidth(const std::string& text, float scale) const {
    float width = 0;
    for (char c : text) {
        auto it = m_glyphs.find(c);
        if (it != m_glyphs.end()) {
            width += it->second.xadvance;
        }
    }
    return width * scale;
}

} // namespace polarclock

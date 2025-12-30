#pragma once

#include <cmath>

namespace math {

struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }

    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        float len = length();
        return len > 0 ? Vec2(x / len, y / len) : Vec2(0, 0);
    }
};

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(const Vec2& v, float z) : x(v.x), y(v.y), z(z) {}

    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
};

struct Vec4 {
    float x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

    Vec3 xyz() const { return Vec3(x, y, z); }
};

struct Mat4 {
    float m[16]; // Column-major order

    Mat4() {
        for (int i = 0; i < 16; ++i) m[i] = 0;
        m[0] = m[5] = m[10] = m[15] = 1; // Identity
    }

    static Mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        Mat4 result;
        result.m[0] = 2.0f / (right - left);
        result.m[5] = 2.0f / (top - bottom);
        result.m[10] = -2.0f / (far - near);
        result.m[12] = -(right + left) / (right - left);
        result.m[13] = -(top + bottom) / (top - bottom);
        result.m[14] = -(far + near) / (far - near);
        result.m[15] = 1.0f;
        return result;
    }

    static Mat4 translate(float x, float y, float z) {
        Mat4 result;
        result.m[12] = x;
        result.m[13] = y;
        result.m[14] = z;
        return result;
    }

    static Mat4 rotate(float angle) { // 2D rotation around Z
        Mat4 result;
        float c = std::cos(angle);
        float s = std::sin(angle);
        result.m[0] = c;
        result.m[1] = s;
        result.m[4] = -s;
        result.m[5] = c;
        return result;
    }

    static Mat4 scale(float sx, float sy, float sz = 1.0f) {
        Mat4 result;
        result.m[0] = sx;
        result.m[5] = sy;
        result.m[10] = sz;
        return result;
    }

    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 16; ++i) result.m[i] = 0;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                for (int k = 0; k < 4; ++k) {
                    result.m[col * 4 + row] += m[k * 4 + row] * other.m[col * 4 + k];
                }
            }
        }
        return result;
    }

    const float* data() const { return m; }
};

// Utility functions
constexpr float PI = 3.14159265358979323846f;
constexpr float TAU = 2.0f * PI;

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float clamp(float x, float min, float max) {
    return x < min ? min : (x > max ? max : x);
}

// Ease-out cubic
inline float easeOut(float t) {
    t = clamp(t, 0.0f, 1.0f);
    float t1 = t - 1.0f;
    return t1 * t1 * t1 + 1.0f;
}

// Convert HSV to RGB (h in [0,1], s in [0,1], v in [0,1])
inline Vec3 hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h * 6, 2) - 1));
    float m = v - c;
    Vec3 rgb;
    if (h < 1.0f/6.0f)      rgb = Vec3(c, x, 0);
    else if (h < 2.0f/6.0f) rgb = Vec3(x, c, 0);
    else if (h < 3.0f/6.0f) rgb = Vec3(0, c, x);
    else if (h < 4.0f/6.0f) rgb = Vec3(0, x, c);
    else if (h < 5.0f/6.0f) rgb = Vec3(x, 0, c);
    else                    rgb = Vec3(c, 0, x);
    return Vec3(rgb.x + m, rgb.y + m, rgb.z + m);
}

} // namespace math

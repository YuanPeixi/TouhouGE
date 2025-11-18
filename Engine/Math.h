#pragma once
#include <cmath>

struct Vec2 {
    float x = 0.0f, y = 0.0f;
    Vec2() = default;
    Vec2(float X, float Y) : x(X), y(Y) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
    float length() const { return std::sqrt(x*x + y*y); }
    Vec2 normalized() const {
        float L = length();
        if (L <= 1e-6f) return Vec2(0.0f, 0.0f);
        return Vec2(x / L, y / L);
    }
};

// 不使用 std::min/std::max,避免与 windows.h 的 min/max 宏冲突
inline float Clamp(float v, float a, float b) {
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float WrapAngle(float a) {
    while (a <= -3.14159265f) a += 6.2831853f;
    while (a >   3.14159265f) a -= 6.2831853f;
    return a;
}
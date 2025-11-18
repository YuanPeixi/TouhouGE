#pragma once
#include <string>
#include <cstdint>

struct Color {
    uint8_t r=0, g=0, b=0, a=255;
    Color() = default;
    Color(uint8_t R, uint8_t G, uint8_t B, uint8_t A=255) : r(R), g(G), b(B), a(A) {}
    static Color Black() { return {0,0,0,255}; }
    static Color White() { return {255,255,255,255}; }
    static Color Red()   { return {220,30,30,255}; }
    static Color Green() { return {30,220,30,255}; }
    static Color Blue()  { return {30,30,220,255}; }
    static Color Yellow(){ return {250,220,50,255}; }
    static Color Cyan()  { return {30,220,220,255}; }
    static Color Magenta(){return {220,30,220,255}; }
    static Color Gray(uint8_t v=128){ return {v,v,v,255}; }
};

struct Size2D { int width=0, height=0; };

struct ITexture {
    virtual ~ITexture() = default;
    virtual int Width() const = 0;
    virtual int Height() const = 0;
};

enum TextAlignFlags {
    AlignLeft   = 1 << 0,
    AlignRight  = 1 << 1,
    AlignTop    = 1 << 2,
    AlignBottom = 1 << 3,
    AlignHCenter= 1 << 4,
    AlignVCenter= 1 << 5,
};

struct IDrawContext {
    virtual ~IDrawContext() = default;
    virtual Size2D GetSize() const = 0;
    virtual void Clear(const Color& c) = 0;
    virtual void DrawCircle(float x, float y, float radius, const Color& fill, const Color& border, float borderWidth=1.0f) = 0;
    virtual void DrawImage(ITexture* tex, float x, float y, float scale=1.0f) = 0;
    virtual void DrawTextW(const std::wstring& text, float x, float y, const Color& color, int alignFlags) = 0;
};

struct IInput {
    virtual ~IInput() = default;
    virtual bool IsKeyDown(int vk) const = 0;
    virtual bool IsMouseDown(int button) const = 0;  // 0: L, 1: R, 2: M
    virtual float MouseX() const = 0;
    virtual float MouseY() const = 0;
};
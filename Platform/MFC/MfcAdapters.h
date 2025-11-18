#pragma once
#include <afxwin.h>
#include <gdiplus.h>
#include <memory>
#include "../../Engine/Interfaces.h"

#pragma comment(lib, "gdiplus.lib")

class GdiplusInit {
public:
    GdiplusInit() {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&token_, &gdiplusStartupInput, nullptr);
    }
    ~GdiplusInit() {
        if (token_) Gdiplus::GdiplusShutdown(token_);
    }
private:
    ULONG_PTR token_ = 0;
};

class MfcTexture : public ITexture {
public:
    explicit MfcTexture(Gdiplus::Image* img) : img_(img) {}
    ~MfcTexture() override { delete img_; }
    int Width() const override { return img_ ? (int)img_->GetWidth() : 0; }
    int Height() const override { return img_ ? (int)img_->GetHeight() : 0; }
    Gdiplus::Image* Get() const { return img_; }
private:
    Gdiplus::Image* img_ = nullptr;
};

// A buffered draw context using GDI+ offscreen bitmap, with logical scaling support
class MfcDrawContext : public IDrawContext {
public:
    MfcDrawContext(CWnd* target, int clientW, int clientH);
    ~MfcDrawContext();

    void Begin();
    void End(CDC* pDC);

    // 设置逻辑坐标尺寸（默认与客户端尺寸相同）。绘制会按 client/logic 比例缩放。
    void SetLogicalSize(int logicalW, int logicalH);

    Size2D GetSize() const override { return { logicalW_, logicalH_ }; }
    void Clear(const Color& c) override;
    void DrawCircle(float x, float y, float radius, const Color& fill, const Color& border, float borderWidth=1.0f) override;
    void DrawImage(ITexture* tex, float x, float y, float scale=1.0f) override;
    void DrawTextW(const std::wstring& text, float x, float y, const Color& color, int alignFlags) override;

private:
    CWnd* target_ = nullptr;

    // 实际缓冲与窗口像素尺寸
    int clientW_ = 0, clientH_ = 0;

    // 逻辑尺寸（供游戏逻辑使用）
    int logicalW_ = 0, logicalH_ = 0;

    // 逻辑->像素缩放
    float scaleX_ = 1.0f, scaleY_ = 1.0f;

    std::unique_ptr<Gdiplus::Bitmap> buffer_;
    std::unique_ptr<Gdiplus::Graphics> g_;
    Gdiplus::SolidBrush brush_{ Gdiplus::Color(255,0,0,0) };
    Gdiplus::Pen pen_{ Gdiplus::Color(255,0,0,0), 1.0f };

    Gdiplus::Color ToGdiColor(const Color& c) {
        return Gdiplus::Color(c.a, c.r, c.g, c.b);
    }
};

class MfcInput : public IInput {
public:
    void HandleKeyDown(WPARAM vk) { if (vk < 256) keys_[(uint8_t)vk] = true; }
    void HandleKeyUp(WPARAM vk) { if (vk < 256) keys_[(uint8_t)vk] = false; }
    void HandleMouseMove(LPARAM lParam) { rawX_ = (float)GET_X_LPARAM(lParam); rawY_ = (float)GET_Y_LPARAM(lParam); }
    void HandleMouseButton(int button, bool down) {
        if (button >= 0 && button < 3) mouse_[button] = down;
    }

    // 设置屏幕像素 -> 逻辑坐标的缩放（例如 800/1200, 600/900）
    void SetMouseToLogicalScale(float sx, float sy) { sx_ = sx; sy_ = sy; }

    bool IsKeyDown(int vk) const override {
        if (vk >= 0 && vk < 256) return keys_[(uint8_t)vk];
        return false;
    }
    bool IsMouseDown(int button) const override { return (button >=0 && button < 3) ? mouse_[button] : false; }
    float MouseX() const override { return rawX_ * sx_; }
    float MouseY() const override { return rawY_ * sy_; }

private:
    bool keys_[256] = {false};
    bool mouse_[3] = {false, false, false};
    float rawX_ = 0.0f, rawY_ = 0.0f; // 像素坐标
    float sx_ = 1.0f, sy_ = 1.0f;     // 屏幕->逻辑 缩放
};
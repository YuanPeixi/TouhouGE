#include "MfcAdapters.h"
#include <string>

MfcDrawContext::MfcDrawContext(CWnd* target, int clientW, int clientH)
    : target_(target), clientW_(clientW), clientH_(clientH),
      logicalW_(clientW), logicalH_(clientH) {
    buffer_ = std::make_unique<Gdiplus::Bitmap>(clientW_, clientH_, PixelFormat32bppPARGB);
    g_ = std::make_unique<Gdiplus::Graphics>(buffer_.get());
    g_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g_->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    SetLogicalSize(clientW_, clientH_);
}

MfcDrawContext::~MfcDrawContext() {}

void MfcDrawContext::SetLogicalSize(int logicalW, int logicalH) {
    logicalW_ = logicalW;
    logicalH_ = logicalH;
    scaleX_ = (logicalW_ > 0) ? (float)clientW_ / (float)logicalW_ : 1.0f;
    scaleY_ = (logicalH_ > 0) ? (float)clientH_ / (float)logicalH_ : 1.0f;
}

void MfcDrawContext::Begin() {
    g_->ResetTransform();
    // 逻辑 -> 像素 缩放
    g_->ScaleTransform(scaleX_, scaleY_);
}

void MfcDrawContext::End(CDC* pDC) {
    if (!pDC) return;
    Gdiplus::Graphics gOut(pDC->m_hDC);
    // 直接把缓冲绘制到窗口（缓冲大小与客户端一致）
    gOut.DrawImage(buffer_.get(), 0, 0);
}

void MfcDrawContext::Clear(const Color& c) {
    g_->Clear(ToGdiColor(c));
}

void MfcDrawContext::DrawCircle(float x, float y, float radius, const Color& fill, const Color& border, float borderWidth) {
    float d = radius * 2.0f;
    Gdiplus::RectF rc(x - radius, y - radius, d, d);
    brush_.SetColor(ToGdiColor(fill));
    pen_.SetColor(ToGdiColor(border));
    pen_.SetWidth(borderWidth);
    g_->FillEllipse(&brush_, rc);
    g_->DrawEllipse(&pen_, rc);
}

void MfcDrawContext::DrawImage(ITexture* tex, float x, float y, float scale) {
    auto* mt = dynamic_cast<MfcTexture*>(tex);
    if (!mt || !mt->Get()) return;
    int w = mt->Width();
    int h = mt->Height();
    float sw = w * scale;
    float sh = h * scale;
    Gdiplus::RectF rc(x - sw*0.5f, y - sh*0.5f, sw, sh);
    g_->DrawImage(mt->Get(), rc);
}

void MfcDrawContext::DrawTextW(const std::wstring& text, float x, float y, const Color& color, int alignFlags) {
    using namespace Gdiplus;
    FontFamily ff(L"Segoe UI");
    // 字号也随缩放变化（在缩放的世界里字体是逻辑尺寸,最后被整体放大）
    Font font(&ff, 18.0f, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(ToGdiColor(color));
    StringFormat fmt;
    fmt.SetFormatFlags(StringFormatFlagsNoWrap);

    // 先测量（注意：当前世界已缩放,MeasureString 返回的是像素空间尺寸）
    RectF layout(0, 0, (REAL)clientW_, (REAL)clientH_);
    RectF bound;
    g_->MeasureString(text.c_str(), (INT)text.size(), &font, layout, &fmt, &bound);

    // 把测量结果换算成逻辑单位
    float wLogic = bound.Width  / scaleX_;
    float hLogic = bound.Height / scaleY_;

    PointF pt{x, y};
    if (alignFlags & AlignRight)    pt.X -= wLogic;
    else if (alignFlags & AlignHCenter) pt.X -= wLogic * 0.5f;

    if (alignFlags & AlignBottom)   pt.Y -= hLogic;
    else if (alignFlags & AlignVCenter) pt.Y -= hLogic * 0.5f;

    g_->DrawString(text.c_str(), (INT)text.size(), &font, pt, &textBrush);
}
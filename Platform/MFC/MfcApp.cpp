#include <afxwin.h>
#include "MfcAdapters.h"
#include "../../Engine/Game.h"

class CGameWnd : public CFrameWnd {
public:
    CGameWnd(int clientW, int clientH, int logicalW, int logicalH)
        : clientW_(clientW), clientH_(clientH),
          draw_(this, clientW, clientH),
          game_(GameConfig{logicalW, logicalH}),
          lastQPC_{0}, qpf_{0} {

        Create(nullptr, _T("Touhou-like Sample (Single Stage) - MFC"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CRect(0, 0, clientW_, clientH_));
        CenterWindow();
        SetTimer(1, 1000/60, nullptr); // 60 FPS

        // 设置逻辑尺寸（例如 800x600）,渲染与输入将按比例缩放
        draw_.SetLogicalSize(logicalW, logicalH);
        input_.SetMouseToLogicalScale((float)logicalW / (float)clientW, (float)logicalH / (float)clientH);

        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        qpf_ = f.QuadPart;
        LARGE_INTEGER c;
        QueryPerformanceCounter(&c);
        lastQPC_ = c.QuadPart;

        auto loadTex = [](const wchar_t* name)->ITexture* {
            auto* img = Gdiplus::Image::FromFile(name, FALSE);
            if (img && img->GetLastStatus() == Gdiplus::Ok) {
                return new MfcTexture(img);
            }
            delete img;
            return nullptr;
        };
        ITexture* playerTex = loadTex(L"player.png");
        ITexture* enemyTex  = loadTex(L"enemy.png");
        game_.SetTextures(playerTex, enemyTex);
    }

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct) {
        if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
            return -1;
        return 0;
    }

    afx_msg void OnPaint() {
        CPaintDC dc(this);
        draw_.Begin();
        game_.Render(draw_);
        draw_.End(&dc);
    }

    afx_msg BOOL OnEraseBkgnd(CDC* /*pDC*/) { return TRUE; } // 双缓冲,不擦背景

    afx_msg void OnTimer(UINT_PTR /*id*/) {
        LARGE_INTEGER c;
        QueryPerformanceCounter(&c);
        double dt = double(c.QuadPart - lastQPC_) / double(qpf_);
        lastQPC_ = c.QuadPart;
        if (dt > 0.05) dt = 0.05;
        game_.Update((float)dt, input_);
        Invalidate(FALSE);
    }

    afx_msg void OnKeyDown(UINT nChar, UINT, UINT) { input_.HandleKeyDown(nChar); }
    afx_msg void OnKeyUp(UINT nChar, UINT, UINT) { input_.HandleKeyUp(nChar); }
    afx_msg void OnLButtonDown(UINT, CPoint) { input_.HandleMouseButton(0, true); SetCapture(); }
    afx_msg void OnLButtonUp(UINT, CPoint) { input_.HandleMouseButton(0, false); ReleaseCapture(); }
    afx_msg void OnRButtonDown(UINT, CPoint) { input_.HandleMouseButton(1, true); SetCapture(); }
    afx_msg void OnRButtonUp(UINT, CPoint) { input_.HandleMouseButton(1, false); ReleaseCapture(); }
    afx_msg void OnMouseMove(UINT, CPoint pt) { input_.HandleMouseMove(MAKELPARAM(pt.x, pt.y)); }

    DECLARE_MESSAGE_MAP()

private:
    int clientW_, clientH_;
    MfcDrawContext draw_;
    MfcInput input_;
    GameEngine game_;
    long long lastQPC_;
    long long qpf_;
};

BEGIN_MESSAGE_MAP(CGameWnd, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

class CGameApp : public CWinApp {
public:
    BOOL InitInstance() override {
        CWinApp::InitInstance();
        static GdiplusInit gdipInit;

        // 设定窗口像素尺寸（可调大）,与逻辑尺寸（固定 800x600）
        const int LOGICAL_W = 800, LOGICAL_H = 600;  // 逻辑坐标空间
        const int CLIENT_W  = 1200, CLIENT_H  = 900; // 实际窗口像素大小,可按需调整

        auto* pWnd = new CGameWnd(CLIENT_W, CLIENT_H, LOGICAL_W, LOGICAL_H);
        m_pMainWnd = pWnd;
        pWnd->ShowWindow(SW_SHOW);
        pWnd->UpdateWindow();
        return TRUE;
    }
} theApp;
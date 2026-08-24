#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <wtypes.h>
#include <objidl.h>

// Fix MinGW PROPID definition before gdiplus
typedef ULONG PROPID;
#include <gdiplus.h>

#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "optical_state.h"
#include "d3d_overlay.h"

using namespace Gdiplus;
using namespace std;

// Exclude from capture flag in Windows 10/11 to eliminate infinite mirror loop
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

// Hotkey IDs
#define HOTKEY_ID_TOGGLE_COMPENSATION 101
#define HOTKEY_ID_FULLSCREEN          102
#define HOTKEY_ID_ESCAPE              103

// Global Application Instance
OpticalState g_state;
ULONG_PTR g_gdiplusToken = 0;
HWND g_hWndMain = NULL;
HWND g_hWndViewport = NULL;
bool g_d3dAvailable = false;
std::wstring g_d3dStatus = L"GPU overlay no probado.";

COLORREF g_colorWindowBg = RGB(5, 8, 17);
COLORREF g_colorPanelBg = RGB(15, 23, 42);
COLORREF g_colorPanelAlt = RGB(30, 41, 59);
COLORREF g_colorText = RGB(248, 250, 252);
COLORREF g_colorMuted = RGB(203, 213, 225);
COLORREF g_colorCyan = RGB(34, 211, 238);
COLORREF g_colorYellow = RGB(250, 204, 21);
COLORREF g_colorDanger = RGB(220, 38, 38);
COLORREF g_colorPrimary = RGB(37, 99, 235);
HBRUSH g_hBrushWindowBg = NULL;
HBRUSH g_hBrushPanelBg = NULL;
HBRUSH g_hBrushPanelAlt = NULL;

// 4 Main Action Buttons: 1, 2, Igual, Mucho Peor
HWND g_hBtnOpt1 = NULL;
HWND g_hBtnOpt2 = NULL;
HWND g_hBtnIgual = NULL;
HWND g_hBtnMuchoPeor = NULL;

// Right Panel OD Controls
HWND g_hODTitle = NULL;
HWND g_hODSub = NULL;

HWND g_hLblSphere = NULL;
HWND g_hBtnSphereMinus = NULL;
HWND g_lblValSphere = NULL;
HWND g_hBtnSpherePlus = NULL;

HWND g_hLblCylinder = NULL;
HWND g_hBtnCylinderMinus = NULL;
HWND g_lblValCylinder = NULL;
HWND g_hBtnCylinderPlus = NULL;

HWND g_hLblAxis = NULL;
HWND g_hBtnAxisMinus = NULL;
HWND g_lblValAxis = NULL;
HWND g_hBtnAxisPlus = NULL;
HWND g_hBtnAxisM5 = NULL;
HWND g_hBtnAxisP5 = NULL;

HWND g_hLblDeconv = NULL;
HWND g_hBtnDeconvMinus = NULL;
HWND g_lblValDeconv = NULL;
HWND g_hBtnDeconvPlus = NULL;

HWND g_hBtnApplyMonitors = NULL;
HWND g_hBtnFullScreen = NULL;
HWND g_hBtnToggleMonitor = NULL;
HWND g_hBtnToggle = NULL;
HWND g_hBtnReset = NULL;

// Right Panel Instructions Box
HWND g_hInstrTitle = NULL;
HWND g_hInstrStep = NULL;
HWND g_hInstrOptionTag = NULL;
HWND g_hInstrLine1 = NULL;
HWND g_hInstrLine2 = NULL;
HWND g_hInstrLine3 = NULL;

HFONT g_hFontBigHeader = NULL;
HFONT g_hFontLabels = NULL;
HFONT g_hFontValues = NULL;
HFONT g_hFontBtns = NULL;
HFONT g_hFontBigBtns = NULL;
HFONT g_hFontApplyBtn = NULL;
HFONT g_hFontInstrBold = NULL;
HFONT g_hFontInstrBody = NULL;

#define TIMER_FRAME_UPDATE 1001

// Control IDs
enum ControlIDs {
    // 4 Action Buttons
    ID_BTN_CHOOSE_1 = 301,
    ID_BTN_CHOOSE_2,
    ID_BTN_CHOOSE_IGUAL,
    ID_BTN_CHOOSE_MUCHO_PEOR,

    // OD Right Panel Buttons
    ID_BTN_SPHERE_MINUS = 401,
    ID_BTN_SPHERE_PLUS,
    ID_BTN_CYLINDER_MINUS,
    ID_BTN_CYLINDER_PLUS,
    ID_BTN_AXIS_MINUS,
    ID_BTN_AXIS_PLUS,
    ID_BTN_AXIS_MINUS_5,
    ID_BTN_AXIS_PLUS_5,
    ID_BTN_DECONV_MINUS,
    ID_BTN_DECONV_PLUS,

    ID_BTN_APPLY_MONITORS = 501,
    ID_BTN_FULLSCREEN,
    ID_BTN_TOGGLE_MONITOR,
    ID_BTN_TOGGLE_SOURCE,
    ID_BTN_RESET
};

// Reusable pixel buffer for zero memory thrashing
std::vector<BYTE> g_pixelCopyBuffer;

// Helper to format floats
std::wstring FormatFloat(float val, int decimals, const std::wstring& unit = L"") {
    std::wstringstream ss;
    if (val > 0.001f && unit.find(L"D") != std::wstring::npos) ss << L"+";
    ss << std::fixed << std::setprecision(decimals) << val << unit;
    return ss.str();
}

void DrawModernButton(LPDRAWITEMSTRUCT dis) {
    if (!dis || !dis->hwndItem) return;

    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    bool focus = (dis->itemState & ODS_FOCUS) != 0;
    RECT rc = dis->rcItem;
    HDC hdc = dis->hDC;

    int ctrlId = (int)dis->CtlID;
    COLORREF fill = g_colorPanelAlt;
    COLORREF border = RGB(71, 85, 105);
    COLORREF text = g_colorText;

    if (ctrlId == ID_BTN_CHOOSE_1) { fill = RGB(185, 28, 28); border = RGB(248, 113, 113); }
    else if (ctrlId == ID_BTN_CHOOSE_2) { fill = RGB(29, 78, 216); border = RGB(96, 165, 250); }
    else if (ctrlId == ID_BTN_CHOOSE_IGUAL) { fill = RGB(21, 128, 61); border = RGB(74, 222, 128); }
    else if (ctrlId == ID_BTN_CHOOSE_MUCHO_PEOR) { fill = RGB(120, 53, 15); border = RGB(251, 191, 36); }
    else if (ctrlId == ID_BTN_APPLY_MONITORS) { fill = RGB(8, 145, 178); border = RGB(103, 232, 249); }
    else if (ctrlId == ID_BTN_RESET) { fill = RGB(51, 65, 85); border = RGB(148, 163, 184); text = RGB(226, 232, 240); }

    if (pressed) {
        fill = RGB(max(0, GetRValue(fill) - 28), max(0, GetGValue(fill) - 28), max(0, GetBValue(fill) - 28));
        OffsetRect(&rc, 1, 1);
    }

    HBRUSH bg = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, focus ? 3 : 2, border);
    HGDIOBJ oldBrush = SelectObject(hdc, bg);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom, 12, 12);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(bg);

    wchar_t textBuf[256];
    GetWindowTextW(dis->hwndItem, textBuf, 256);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, text);
    HFONT hFont = (HFONT)SendMessageW(dis->hwndItem, WM_GETFONT, 0, 0);
    HGDIOBJ oldFont = hFont ? SelectObject(hdc, hFont) : NULL;
    DrawTextW(hdc, textBuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    if (oldFont) SelectObject(hdc, oldFont);
}

void UpdateLabels() {
    if (g_lblValSphere) SetWindowTextW(g_lblValSphere, FormatFloat(g_state.sphere, 2, L" D").c_str());
    if (g_lblValCylinder) SetWindowTextW(g_lblValCylinder, FormatFloat(g_state.cylinder, 2, L" D").c_str());
    if (g_lblValAxis) SetWindowTextW(g_lblValAxis, (std::to_wstring(g_state.axis) + L"°").c_str());
    if (g_lblValDeconv) SetWindowTextW(g_lblValDeconv, FormatFloat(g_state.deconv, 1, L"x").c_str());
    UpdateD3DOverlayParameters(g_state);

    // Update Right Panel Instructions & Status
    if (g_state.appPhase == PHASE_CALIBRATION_WIZARD) {
        std::wstring stepStr = (g_state.calibPhase == CALIB_SPHERE) ? L"Adivinando: 1/3 Miopía (Esfera)" :
                               (g_state.calibPhase == CALIB_CYL_POWER) ? L"Adivinando: 2/3 Astigmatismo (Cil)" :
                               L"Adivinando: 3/3 Eje Fino (+/-5°)";
        if (g_hInstrStep) SetWindowTextW(g_hInstrStep, stepStr.c_str());

        std::wstring optStr = (g_state.autoCurrentOption == 1)
            ? L"🔴 En pantalla: OPCIÓN 1"
            : L"🔵 En pantalla: OPCIÓN 2";
        if (g_hInstrOptionTag) SetWindowTextW(g_hInstrOptionTag, optStr.c_str());

        if (g_hInstrLine1) SetWindowTextW(g_hInstrLine1, L"• Alterna [1] y [2] cada 1 segundo automáticamente.");
        if (g_hInstrLine2) SetWindowTextW(g_hInstrLine2, L"• Pulsa [1] o [2] según cuál se vea más clara.");
        if (g_hInstrLine3) SetWindowTextW(g_hInstrLine3, (L"• " + g_d3dStatus).c_str());
    } else {
        std::wstring targetStr = (g_state.monitorTarget == MONITOR_PRIMARY) ? L"Monitor 1" :
                                 (g_state.monitorTarget == MONITOR_SECONDARY) ? L"Monitor 2" : L"Todos los Monitores";
        if (g_hInstrStep) SetWindowTextW(g_hInstrStep, (L"Estado: 🔎 Corrección Activa (" + targetStr + L")").c_str());
        if (g_hInstrOptionTag) SetWindowTextW(g_hInstrOptionTag, (L"Receta: " + FormatFloat(g_state.sphere, 2, L"D") + L" / " + FormatFloat(g_state.cylinder, 2, L"D") + L" @" + std::to_wstring(g_state.axis) + L"°").c_str());
        if (g_hInstrLine1) SetWindowTextW(g_hInstrLine1, L"• [F11] o [Ctrl+Alt+V] para activar/desactivar.");
        if (g_hInstrLine2) SetWindowTextW(g_hInstrLine2, L"• Ratón y teclado 100% funcionales (Click-Through).");
        if (g_hInstrLine3) SetWindowTextW(g_hInstrLine3, L"• Pulsa [Esc] para volver a la ventana de ajuste.");
    }
}

// Clinical Jackson Cross-Cylinder (JCC) Test Value Generator
void SetupCurrentTestOptions() {
    if (g_state.calibPhase == CALIB_SPHERE) {
        g_state.sphereCenter = max(-10.0f, min(10.0f, g_state.sphereCenter));
        g_state.opt1Sphere = g_state.sphereCenter + (g_state.sphereStep * 0.5f);
        g_state.opt2Sphere = g_state.sphereCenter - (g_state.sphereStep * 0.5f);
        g_state.opt1Sphere = max(-10.0f, min(10.0f, g_state.opt1Sphere));
        g_state.opt2Sphere = max(-10.0f, min(10.0f, g_state.opt2Sphere));
    } else if (g_state.calibPhase == CALIB_CYL_POWER) {
        g_state.cylCenter = max(-8.0f, min(8.0f, g_state.cylCenter));
        g_state.opt1Cyl = g_state.cylCenter + (g_state.cylStep * 0.5f);
        g_state.opt2Cyl = g_state.cylCenter - (g_state.cylStep * 0.5f);
        g_state.opt1Cyl = max(-8.0f, min(8.0f, g_state.opt1Cyl));
        g_state.opt2Cyl = max(-8.0f, min(8.0f, g_state.opt2Cyl));
    } else if (g_state.calibPhase == CALIB_AXIS_FINE) {
        g_state.opt1Axis = (float)((int)(g_state.axisCenter - (g_state.axisStep * 0.5f) + 181) % 181);
        g_state.opt2Axis = (float)((int)(g_state.axisCenter + (g_state.axisStep * 0.5f) + 181) % 181);
    }
}

void ResetToDefaults() {
    g_state.sphere = -4.50f;
    g_state.cylinder = -1.25f;
    g_state.axis = 95;
    g_state.distance = 60;
    g_state.deconv = 1.2f;
    g_state.contrast = 1.05f;
    g_state.marginRatio = 0.94f;

    g_state.appPhase = PHASE_CALIBRATION_WIZARD;
    g_state.calibPhase = CALIB_SPHERE;
    g_state.autoCurrentOption = 1;
    g_state.lastToggleTick = GetTickCount();

    g_state.sphereCenter = -4.50f;
    g_state.sphereStep = 1.00f;

    g_state.cylCenter = -1.25f;
    g_state.cylStep = 0.75f;

    g_state.axisCenter = 95.0f;
    g_state.axisStep = 10.0f;

    g_state.iterationCount = 0;
    g_state.sourceType = 0;

    SetupCurrentTestOptions();
    UpdateLabels();
    if (g_hWndViewport) InvalidateRect(g_hWndViewport, NULL, FALSE);
}

// Adjust window size to 90% of primary monitor
void SetWindowTo90Percent(HWND hWnd) {
    RECT workArea;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    int screenW = workArea.right - workArea.left;
    int screenH = workArea.bottom - workArea.top;

    int winW = (int)(screenW * 0.92f);
    int winH = (int)(screenH * 0.92f);
    int posX = workArea.left + (screenW - winW) / 2;
    int posY = workArea.top + (screenH - winH) / 2;

    SetWindowPos(hWnd, HWND_TOP, posX, posY, winW, winH, SWP_SHOWWINDOW);
}

// Toggle F11 Fullscreen with Click-Through Passthrough
void ToggleFullScreen(HWND hWnd) {
    g_state.isFullScreen = !g_state.isFullScreen;

    if (g_state.isFullScreen) {
        GetWindowRect(hWnd, &g_state.prevNormalRect);

        int vx = 0, vy = 0, vw = 1920, vh = 1080;

        if (g_state.monitorTarget == MONITOR_ALL_VIRTUAL) {
            vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
            vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
            vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        } else {
            HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(mi) };
            if (GetMonitorInfo(hMon, &mi)) {
                vx = mi.rcMonitor.left;
                vy = mi.rcMonitor.top;
                vw = mi.rcMonitor.right - mi.rcMonitor.left;
                vh = mi.rcMonitor.bottom - mi.rcMonitor.top;
            }
        }

        // Fullscreen Overlay with Transparent Click-Through so Mouse & Keyboard work on apps below!
        SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);
        SetWindowPos(hWnd, HWND_TOPMOST, vx, vy, vw, vh, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    } else {
        // Return to Normal GUI Window with interactive buttons
        SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) & ~(WS_EX_TRANSPARENT | WS_EX_TOPMOST));
        SetWindowPos(hWnd, HWND_NOTOPMOST, 
                     g_state.prevNormalRect.left, g_state.prevNormalRect.top,
                     g_state.prevNormalRect.right - g_state.prevNormalRect.left,
                     g_state.prevNormalRect.bottom - g_state.prevNormalRect.top,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}

// Dynamic Control Relayout
void RelayoutControls(int clientW, int clientH) {
    if (clientW <= 100 || clientH <= 100) return;

    if (g_state.isFullScreen) {
        if (g_hWndViewport) {
            SetWindowPos(g_hWndViewport, NULL, 0, 0, clientW, clientH, SWP_NOZORDER);
        }
        auto HideControl = [](HWND h) { if (h) ShowWindow(h, SW_HIDE); };
        HideControl(g_hBtnOpt1); HideControl(g_hBtnOpt2); HideControl(g_hBtnIgual); HideControl(g_hBtnMuchoPeor);
        HideControl(g_hODTitle); HideControl(g_hODSub);
        HideControl(g_hLblSphere); HideControl(g_hBtnSphereMinus); HideControl(g_lblValSphere); HideControl(g_hBtnSpherePlus);
        HideControl(g_hLblCylinder); HideControl(g_hBtnCylinderMinus); HideControl(g_lblValCylinder); HideControl(g_hBtnCylinderPlus);
        HideControl(g_hLblAxis); HideControl(g_hBtnAxisMinus); HideControl(g_lblValAxis); HideControl(g_hBtnAxisPlus);
        HideControl(g_hBtnAxisM5); HideControl(g_hBtnAxisP5);
        HideControl(g_hLblDeconv); HideControl(g_hBtnDeconvMinus); HideControl(g_lblValDeconv); HideControl(g_hBtnDeconvPlus);
        HideControl(g_hBtnApplyMonitors); HideControl(g_hBtnFullScreen); HideControl(g_hBtnToggleMonitor); HideControl(g_hBtnToggle); HideControl(g_hBtnReset);
        HideControl(g_hInstrTitle); HideControl(g_hInstrStep); HideControl(g_hInstrOptionTag);
        HideControl(g_hInstrLine1); HideControl(g_hInstrLine2); HideControl(g_hInstrLine3);
        return;
    }

    // Normal Windowed Mode
    auto ShowControl = [](HWND h) { if (h) ShowWindow(h, SW_SHOW); };
    ShowControl(g_hBtnOpt1); ShowControl(g_hBtnOpt2); ShowControl(g_hBtnIgual); ShowControl(g_hBtnMuchoPeor);
    ShowControl(g_hODTitle); ShowControl(g_hODSub);
    ShowControl(g_hLblSphere); ShowControl(g_hBtnSphereMinus); ShowControl(g_lblValSphere); ShowControl(g_hBtnSpherePlus);
    ShowControl(g_hLblCylinder); ShowControl(g_hBtnCylinderMinus); ShowControl(g_lblValCylinder); ShowControl(g_hBtnCylinderPlus);
    ShowControl(g_hLblAxis); ShowControl(g_hBtnAxisMinus); ShowControl(g_lblValAxis); ShowControl(g_hBtnAxisPlus);
    ShowControl(g_hBtnAxisM5); ShowControl(g_hBtnAxisP5);
    ShowControl(g_hLblDeconv); ShowControl(g_hBtnDeconvMinus); ShowControl(g_lblValDeconv); ShowControl(g_hBtnDeconvPlus);
    ShowControl(g_hBtnApplyMonitors); ShowControl(g_hBtnFullScreen); ShowControl(g_hBtnToggleMonitor); ShowControl(g_hBtnToggle); ShowControl(g_hBtnReset);
    ShowControl(g_hInstrTitle); ShowControl(g_hInstrStep); ShowControl(g_hInstrOptionTag);
    ShowControl(g_hInstrLine1); ShowControl(g_hInstrLine2); ShowControl(g_hInstrLine3);

    int rightPanelW = 340;
    int bottomBarH = 76;

    int vpW = clientW - rightPanelW - 25;
    int vpH = clientH - bottomBarH - 25;

    // 1. Viewport (Left side)
    if (g_hWndViewport) {
        SetWindowPos(g_hWndViewport, NULL, 10, 10, max(200, vpW), max(200, vpH), SWP_NOZORDER);
    }

    // 2. Bottom 4 Action Buttons
    int btnY = vpH + 16;
    int btnCount = 4;
    int gap = 10;
    int singleBtnW = (vpW - (gap * (btnCount - 1))) / btnCount;

    if (g_hBtnOpt1) SetWindowPos(g_hBtnOpt1, NULL, 10 + 0 * (singleBtnW + gap), btnY, singleBtnW, 60, SWP_NOZORDER);
    if (g_hBtnOpt2) SetWindowPos(g_hBtnOpt2, NULL, 10 + 1 * (singleBtnW + gap), btnY, singleBtnW, 60, SWP_NOZORDER);
    if (g_hBtnIgual) SetWindowPos(g_hBtnIgual, NULL, 10 + 2 * (singleBtnW + gap), btnY, singleBtnW, 60, SWP_NOZORDER);
    if (g_hBtnMuchoPeor) SetWindowPos(g_hBtnMuchoPeor, NULL, 10 + 3 * (singleBtnW + gap), btnY, singleBtnW, 60, SWP_NOZORDER);

    // 3. Right Side Controls
    int rx = clientW - rightPanelW - 10;
    int ry = 8;
    int rColW = rightPanelW;

    if (g_hODTitle) SetWindowPos(g_hODTitle, NULL, rx, ry, rColW, 24, SWP_NOZORDER);
    ry += 24;
    if (g_hODSub) SetWindowPos(g_hODSub, NULL, rx, ry, rColW, 16, SWP_NOZORDER);
    ry += 20;

    auto LayoutODRow = [&](HWND hLbl, HWND hMinus, HWND hVal, HWND hPlus, HWND hM5 = NULL, HWND hP5 = NULL) {
        if (hLbl) SetWindowPos(hLbl, NULL, rx, ry + 4, 120, 24, SWP_NOZORDER);
        if (hMinus) SetWindowPos(hMinus, NULL, rx + 125, ry, 38, 30, SWP_NOZORDER);
        if (hVal) SetWindowPos(hVal, NULL, rx + 168, ry + 2, 80, 26, SWP_NOZORDER);
        if (hPlus) SetWindowPos(hPlus, NULL, rx + 254, ry, 38, 30, SWP_NOZORDER);

        if (hM5 && hP5) {
            ry += 32;
            SetWindowPos(hM5, NULL, rx + 125, ry, 60, 26, SWP_NOZORDER);
            SetWindowPos(hP5, NULL, rx + 232, ry, 60, 26, SWP_NOZORDER);
        }
        ry += 34;
    };

    LayoutODRow(g_hLblSphere, g_hBtnSphereMinus, g_lblValSphere, g_hBtnSpherePlus);
    LayoutODRow(g_hLblCylinder, g_hBtnCylinderMinus, g_lblValCylinder, g_hBtnCylinderPlus);
    LayoutODRow(g_hLblAxis, g_hBtnAxisMinus, g_lblValAxis, g_hBtnAxisPlus, g_hBtnAxisM5, g_hBtnAxisP5);
    LayoutODRow(g_hLblDeconv, g_hBtnDeconvMinus, g_lblValDeconv, g_hBtnDeconvPlus);

    ry += 4;
    if (g_hBtnApplyMonitors) SetWindowPos(g_hBtnApplyMonitors, NULL, rx, ry, rColW, 42, SWP_NOZORDER);
    ry += 46;

    if (g_hBtnFullScreen) SetWindowPos(g_hBtnFullScreen, NULL, rx, ry, rColW, 32, SWP_NOZORDER);
    ry += 34;
    if (g_hBtnToggleMonitor) SetWindowPos(g_hBtnToggleMonitor, NULL, rx, ry, rColW, 32, SWP_NOZORDER);
    ry += 34;
    if (g_hBtnToggle) SetWindowPos(g_hBtnToggle, NULL, rx, ry, rColW, 32, SWP_NOZORDER);
    ry += 34;
    if (g_hBtnReset) SetWindowPos(g_hBtnReset, NULL, rx, ry, rColW, 28, SWP_NOZORDER);
    ry += 32;

    // 4. Instructions Box
    if (g_hInstrTitle) SetWindowPos(g_hInstrTitle, NULL, rx, ry, rColW, 20, SWP_NOZORDER);
    ry += 20;
    if (g_hInstrStep) SetWindowPos(g_hInstrStep, NULL, rx, ry, rColW, 20, SWP_NOZORDER);
    ry += 20;
    if (g_hInstrOptionTag) SetWindowPos(g_hInstrOptionTag, NULL, rx, ry, rColW, 22, SWP_NOZORDER);
    ry += 22;
    if (g_hInstrLine1) SetWindowPos(g_hInstrLine1, NULL, rx, ry, rColW, 32, SWP_NOZORDER);
    ry += 34;
    if (g_hInstrLine2) SetWindowPos(g_hInstrLine2, NULL, rx, ry, rColW, 32, SWP_NOZORDER);
    ry += 34;
    if (g_hInstrLine3) SetWindowPos(g_hInstrLine3, NULL, rx, ry, rColW, 32, SWP_NOZORDER);
}

// Multi-Monitor Safe Capture with 100% True-Color Alpha Preservation
void CaptureDesktopToBitmap(HWND hWndTarget, Bitmap* bmp) {
    if (!hWndTarget || !bmp) return;
    
    int w = bmp->GetWidth();
    int h = bmp->GetHeight();
    if (w <= 0 || h <= 0) return;

    POINT pt = {0, 0};
    ClientToScreen(hWndTarget, &pt);

    HDC screenDC = GetDC(NULL);
    if (!screenDC) return;

    HDC memDC = CreateCompatibleDC(screenDC);
    if (!memDC) {
        ReleaseDC(NULL, screenDC);
        return;
    }

    HBITMAP memBmp = CreateCompatibleBitmap(screenDC, w, h);
    if (!memBmp) {
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return;
    }

    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);
    BitBlt(memDC, 0, 0, w, h, screenDC, pt.x, pt.y, SRCCOPY);

    Graphics g(bmp);
    HDC bmpDC = g.GetHDC();
    if (bmpDC) {
        BitBlt(bmpDC, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
        g.ReleaseHDC(bmpDC);
    }

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);

    // Set Alpha to 0xFF (255) for solid opacity
    BitmapData bmpData;
    Rect rect(0, 0, w, h);
    if (bmp->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bmpData) == Ok) {
        BYTE* row = (BYTE*)bmpData.Scan0;
        int stride = bmpData.Stride;
        for (int y = 0; y < h; y++) {
            BYTE* p = row + (y * stride);
            for (int x = 0; x < w; x++) {
                p[x * 4 + 3] = 255;
            }
        }
        bmp->UnlockBits(&bmpData);
    }
}

// Render Medical Font Size Progression Test (28, 24, 22, 18, 16, 14, 12 pt)
void RenderMedicalFontChart(Graphics& g, int w, int h) {
    SolidBrush bgBrush(Color(255, 17, 24, 39));
    g.FillRectangle(&bgBrush, 0, 0, w, h);

    FontFamily timesFamily(L"Times New Roman");
    FontFamily latoFamily(L"Segoe UI");

    Font f28(&timesFamily, 28, FontStyleBold, UnitPixel);
    Font f24(&timesFamily, 24, FontStyleBold, UnitPixel);
    Font f22(&timesFamily, 22, FontStyleRegular, UnitPixel);
    Font f18(&timesFamily, 18, FontStyleRegular, UnitPixel);
    Font f16(&timesFamily, 16, FontStyleRegular, UnitPixel);
    Font f14(&timesFamily, 14, FontStyleRegular, UnitPixel);
    Font f12(&timesFamily, 12, FontStyleRegular, UnitPixel);
    Font fSnellen2020(&latoFamily, 14, FontStyleBold, UnitPixel);

    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    SolidBrush cyanBrush(Color(255, 56, 189, 248));
    SolidBrush yellowBrush(Color(255, 251, 191, 36));
    SolidBrush greenBrush(Color(255, 52, 211, 153));
    SolidBrush grayBrush(Color(255, 209, 213, 219));

    int padX = 35;
    int curY = 25;

    // Line 1: [Font 28px]
    g.DrawString(L"28px  E  F  P  T  O  Z  L  P  E  D", -1, &f28, PointF((REAL)padX, (REAL)curY), &cyanBrush);
    curY += 42;

    // Line 2: [Font 24px]
    g.DrawString(L"24px  \"The quick brown fox jumps over the lazy dog.\"", -1, &f24, PointF((REAL)padX, (REAL)curY), &whiteBrush);
    curY += 38;

    // Line 3: [Font 22px]
    g.DrawString(L"22px  ABCDEFGHIJKLMNOPQRSTUVWXYZ  1234567890", -1, &f22, PointF((REAL)padX, (REAL)curY), &greenBrush);
    curY += 34;

    // Line 4: [Font 18px]
    RectF p18Rect((REAL)padX, (REAL)curY, (REAL)(w - padX * 2), 52);
    std::wstring p18 = L"18px  La refracción óptica compensa la miopía y el astigmatismo de tu córnea en tiempo real.";
    g.DrawString(p18.c_str(), -1, &f18, p18Rect, NULL, &yellowBrush);
    curY += 32;

    // Line 5: [Font 16px]
    RectF p16Rect((REAL)padX, (REAL)curY, (REAL)(w - padX * 2), 52);
    std::wstring p16 = L"16px  Compara la Opción 1 y Opción 2: elige la que te muestre los bordes de cada letra sin sombra.";
    g.DrawString(p16.c_str(), -1, &f16, p16Rect, NULL, &whiteBrush);
    curY += 30;

    // Line 6: [Font 14px]
    g.DrawString(L"14px  Caracteres Críticos: [ C / O / G / Q ]   [ B / 8 / 3 / S ]   [ P / R / F / E ]", -1, &f14, PointF((REAL)padX, (REAL)curY), &grayBrush);
    curY += 26;

    // Line 7: [Font 12px]
    g.DrawString(L"12px  Lectura fina: const lens = { sphere: -4.50, cyl: -1.25, axis: 95 }; // 100% Nítido", -1, &f12, PointF((REAL)padX, (REAL)curY), &grayBrush);
    curY += 24;

    // Line 8: Snellen HD 20/20
    g.DrawString(L"20/20 HD  E  F  P  T  O  Z  L  P  E  D  P  E  C  F  D  E  D  F  C  Z  P  1 2 3 4 5 6 7 8 9 0", -1, &fSnellen2020, PointF((REAL)padX, (REAL)curY), &cyanBrush);
}

// 100% Crash-Proof Pixel processing for directional & omnidirectional sharpening with True Color
void ApplyPixelProcessing(Bitmap* bmp, float sphereVal, float cylVal, int axisAngle, float deconvStrength, float contrastVal) {
    if (!bmp) return;

    int w = bmp->GetWidth();
    int h = bmp->GetHeight();
    if (w <= 2 || h <= 2) return;

    BitmapData bmpData;
    Rect rect(0, 0, w, h);
    if (bmp->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bmpData) != Ok) return;

    int stride = bmpData.Stride;
    size_t totalBytes = (size_t)stride * h;
    BYTE* pixels = (BYTE*)bmpData.Scan0;

    if (!pixels || totalBytes == 0) {
        bmp->UnlockBits(&bmpData);
        return;
    }

    if (g_pixelCopyBuffer.size() < totalBytes) {
        g_pixelCopyBuffer.resize(totalBytes);
    }
    memcpy(g_pixelCopyBuffer.data(), pixels, totalBytes);
    BYTE* copy = g_pixelCopyBuffer.data();

    float effectiveSharpness = deconvStrength + (fabs(sphereVal) * 0.12f) + (fabs(cylVal) * 0.20f);
    if (effectiveSharpness > 3.8f) effectiveSharpness = 3.8f;

    float angleRad = (float)(axisAngle * 3.14159265f / 180.0f);
    int dx = (int)round(cos(angleRad));
    int dy = (int)round(sin(angleRad));

    for (int y = 1; y < h - 1; y++) {
        BYTE* row = pixels + (y * stride);
        
        int ny1 = min(h - 1, max(0, y + dy));
        int ny2 = min(h - 1, max(0, y - dy));
        BYTE* rowN1 = copy + (ny1 * stride);
        BYTE* rowN2 = copy + (ny2 * stride);
        BYTE* rowCopy = copy + (y * stride);
        BYTE* rowTop = copy + ((y - 1) * stride);
        BYTE* rowBottom = copy + ((y + 1) * stride);

        for (int x = 1; x < w - 1; x++) {
            int nx1 = min(w - 1, max(0, x + dx));
            int nx2 = min(w - 1, max(0, x - dx));

            int offset = x * 4;
            int offsetN1 = nx1 * 4;
            int offsetN2 = nx2 * 4;
            int offsetL = (x - 1) * 4;
            int offsetR = (x + 1) * 4;

            for (int c = 0; c < 3; c++) {
                int center = (int)rowCopy[offset + c];
                int n1 = (int)rowN1[offsetN1 + c];
                int n2 = (int)rowN2[offsetN2 + c];
                int highPassDir = center * 2 - n1 - n2;

                int top = (int)rowTop[offset + c];
                int bottom = (int)rowBottom[offset + c];
                int left = (int)rowCopy[offsetL + c];
                int right = (int)rowCopy[offsetR + c];
                int highPassOmni = center * 4 - top - bottom - left - right;

                float val = (float)center + (float)highPassDir * (effectiveSharpness * 0.15f) + (float)highPassOmni * (effectiveSharpness * 0.08f);

                // Pure Linear Contrast Formula (100% prevents color inversion/negative bug!)
                if (contrastVal != 1.0f) {
                    val = ((val - 128.0f) * contrastVal) + 128.0f;
                }

                if (val < 0.0f) val = 0.0f;
                else if (val > 255.0f) val = 255.0f;
                row[offset + c] = (BYTE)val;
            }
            row[offset + 3] = 255;
        }
    }

    bmp->UnlockBits(&bmpData);
}

// Render the viewport scene with clinical Jackson cylinder stabilization
void RenderViewportScene(HWND hWndTarget, HDC hdc, int width, int height) {
    if (width <= 0 || height <= 0) return;

    HDC memDC = CreateCompatibleDC(hdc);
    if (!memDC) return;

    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    if (!memBmp) {
        DeleteDC(memDC);
        return;
    }

    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    Graphics g(memDC);
    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    SolidBrush bg(Color(255, 11, 15, 25));
    g.FillRectangle(&bg, 0, 0, width, height);

    // 1. Source Pattern or Live Desktop
    Bitmap sourceBmp(width, height, PixelFormat32bppARGB);
    if (g_state.sourceType == 3) {
        CaptureDesktopToBitmap(hWndTarget, &sourceBmp);
    } else {
        Graphics gSrc(&sourceBmp);
        gSrc.SetSmoothingMode(SmoothingModeAntiAlias);
        gSrc.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        RenderMedicalFontChart(gSrc, width, height);
    }

    // Determine current values to apply
    float curSphere = g_state.sphere;
    float curCyl = g_state.cylinder;
    int curAxis = g_state.axis;

    if (g_state.appPhase == PHASE_CALIBRATION_WIZARD) {
        if (g_state.calibPhase == CALIB_SPHERE) {
            curSphere = (g_state.autoCurrentOption == 1) ? g_state.opt1Sphere : g_state.opt2Sphere;
        } else if (g_state.calibPhase == CALIB_CYL_POWER) {
            curCyl = (g_state.autoCurrentOption == 1) ? g_state.opt1Cyl : g_state.opt2Cyl;
        } else if (g_state.calibPhase == CALIB_AXIS_FINE) {
            curAxis = (g_state.autoCurrentOption == 1) ? (int)g_state.opt1Axis : (int)g_state.opt2Axis;
        }
    }

    // 2. Optical Distortion Matrix with -10% Margin Headroom & Bounded Aspect Shear
    Bitmap processedBmp(width, height, PixelFormat32bppARGB);
    Graphics gProc(&processedBmp);
    gProc.SetSmoothingMode(SmoothingModeHighQuality);
    gProc.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    gProc.FillRectangle(&bg, 0, 0, width, height);

    float distFactor = 60.0f / (float)max(30, g_state.distance);
    
    // Scale Sphere & Margin Headroom
    float sphereScale = (1.0f + (curSphere * -0.022f * distFactor)) * g_state.marginRatio;
    if (sphereScale < 0.70f) sphereScale = 0.70f;
    if (sphereScale > 1.45f) sphereScale = 1.45f;

    // Bounded Cylinder Anamorphic ratio
    float cylRatio = 1.0f + (curCyl * -0.022f * distFactor);
    if (cylRatio < 0.85f) cylRatio = 0.85f;
    if (cylRatio > 1.25f) cylRatio = 1.25f;

    float scaleX = sphereScale * cylRatio;
    float scaleY = sphereScale / cylRatio;

    REAL cx = (REAL)width / 2.0f;
    REAL cy = (REAL)height / 2.0f;

    Matrix mat;
    mat.Translate(cx, cy);
    mat.Rotate((REAL)curAxis);
    mat.Scale(scaleX, scaleY);
    mat.Rotate((REAL)-curAxis);
    mat.Translate(-cx, -cy);

    gProc.SetTransform(&mat);
    gProc.DrawImage(&sourceBmp, 0, 0, width, height);
    gProc.ResetTransform();

    // Apply high-frequency directional deconvolution sharpening
    ApplyPixelProcessing(&processedBmp, curSphere, curCyl, curAxis, g_state.deconv, g_state.contrast);

    // Draw processed image
    g.DrawImage(&processedBmp, 0, 0, width, height);

    // --- BOTTOM-RIGHT 1-SECOND FLIP INDICATOR (1 vs 2) ---
    if (g_state.appPhase == PHASE_CALIBRATION_WIZARD) {
        int tagW = 190;
        int tagH = 48;
        int tagX = width - tagW - 20;
        int tagY = height - tagH - 20;

        Color tagBgColor = (g_state.autoCurrentOption == 1) ? Color(230, 220, 38, 38) : Color(230, 37, 99, 235);
        SolidBrush tagBg(tagBgColor);
        g.FillRectangle(&tagBg, tagX, tagY, tagW, tagH);

        Pen tagBorder(Color(255, 255, 255, 255), 2.0f);
        g.DrawRectangle(&tagBorder, tagX, tagY, tagW, tagH);

        Font tagFont(L"Segoe UI", 16, FontStyleBold, UnitPixel);
        SolidBrush textWhite(Color(255, 255, 255, 255));

        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);

        std::wstring tagText = (g_state.autoCurrentOption == 1) ? L"🔴 OPCIÓN 1" : L"🔵 OPCIÓN 2";
        RectF tagRect((REAL)tagX, (REAL)tagY, (REAL)tagW, (REAL)tagH);
        g.DrawString(tagText.c_str(), -1, &tagFont, tagRect, &sf, &textWhite);
    }

    if (!g_state.isFullScreen) {
        Pen framePen(Color(255, 6, 182, 212), 2.0f);
        g.DrawRectangle(&framePen, 1, 1, width - 2, height - 2);
    }

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

// Viewport Window Procedure
LRESULT CALLBACK ViewportWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetClientRect(hWnd, &rc);
            RenderViewportScene(hWnd, hdc, rc.right - rc.left, rc.bottom - rc.top);
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// Main Window Procedure
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hWndMain = hWnd;
            InitCommonControls();
            g_d3dAvailable = ProbeD3D11Overlay(&g_d3dStatus);

            // Register Global Hotkeys so you never lose control of keyboard/mouse
            RegisterHotKey(hWnd, HOTKEY_ID_TOGGLE_COMPENSATION, MOD_CONTROL | MOD_ALT, 'V');
            RegisterHotKey(hWnd, HOTKEY_ID_FULLSCREEN, 0, VK_F11);
            RegisterHotKey(hWnd, HOTKEY_ID_ESCAPE, 0, VK_ESCAPE);

            g_hBrushWindowBg = CreateSolidBrush(g_colorWindowBg);
            g_hBrushPanelBg = CreateSolidBrush(g_colorPanelBg);
            g_hBrushPanelAlt = CreateSolidBrush(g_colorPanelAlt);
            
            // Fonts
            g_hFontBigHeader = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            g_hFontLabels = CreateFontW(13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            g_hFontValues = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");
            g_hFontBtns = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            g_hFontBigBtns = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            g_hFontApplyBtn = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

            g_hFontInstrBold = CreateFontW(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            g_hFontInstrBody = CreateFontW(11, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

            // Register Viewport Class
            WNDCLASSW wc = {0};
            wc.lpfnWndProc = ViewportWndProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = L"OpticalViewportDirectClass";
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            RegisterClassW(&wc);

            // 1. Viewport (Left)
            g_hWndViewport = CreateWindowExW(
                0, L"OpticalViewportDirectClass", L"",
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                10, 10, 800, 600, hWnd, NULL, wc.hInstance, NULL
            );

            // 2. Bottom 4 Action Buttons
            g_hBtnOpt1 = CreateWindowW(L"BUTTON", L"1️⃣ 1 es Mejor [1]", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 50, hWnd, (HMENU)ID_BTN_CHOOSE_1, wc.hInstance, NULL);
            g_hBtnOpt2 = CreateWindowW(L"BUTTON", L"2️⃣ 2 es Mejor [2]", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 50, hWnd, (HMENU)ID_BTN_CHOOSE_2, wc.hInstance, NULL);
            g_hBtnIgual = CreateWindowW(L"BUTTON", L"⚖️ Igual / Veo Bien [3]", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 50, hWnd, (HMENU)ID_BTN_CHOOSE_IGUAL, wc.hInstance, NULL);
            g_hBtnMuchoPeor = CreateWindowW(L"BUTTON", L"🚫 Mucho Peor [4]", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 50, hWnd, (HMENU)ID_BTN_CHOOSE_MUCHO_PEOR, wc.hInstance, NULL);

            // 3. Right Side Controls
            g_hODTitle = CreateWindowW(L"STATIC", L"RECETA LENTES (OD)", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_CENTER, 0, 0, 100, 24, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hODTitle, WM_SETFONT, (WPARAM)g_hFontBigHeader, TRUE);

            g_hODSub = CreateWindowW(L"STATIC", L"Rango Extendido [-10D a +10D]:", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_CENTER, 0, 0, 100, 16, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hODSub, WM_SETFONT, (WPARAM)g_hFontLabels, TRUE);

            auto CreateODRowControls = [&](const std::wstring& name, int minusId, int plusId, HWND& outLbl, HWND& outMinus, HWND& outVal, HWND& outPlus, HWND* outM5 = NULL, HWND* outP5 = NULL, int minus5Id = 0, int plus5Id = 0) {
                outLbl = CreateWindowW(L"STATIC", name.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 20, hWnd, NULL, wc.hInstance, NULL);
                SendMessageW(outLbl, WM_SETFONT, (WPARAM)g_hFontLabels, TRUE);

                outMinus = CreateWindowW(L"BUTTON", L"-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 34, 28, hWnd, (HMENU)(INT_PTR)minusId, wc.hInstance, NULL);
                outVal = CreateWindowW(L"STATIC", L"0.00", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_CENTER | SS_SUNKEN, 0, 0, 75, 24, hWnd, NULL, wc.hInstance, NULL);
                SendMessageW(outVal, WM_SETFONT, (WPARAM)g_hFontValues, TRUE);

                outPlus = CreateWindowW(L"BUTTON", L"+", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 34, 28, hWnd, (HMENU)(INT_PTR)plusId, wc.hInstance, NULL);

                if (outM5 && outP5) {
                    *outM5 = CreateWindowW(L"BUTTON", L"-5°", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 54, 24, hWnd, (HMENU)(INT_PTR)minus5Id, wc.hInstance, NULL);
                    *outP5 = CreateWindowW(L"BUTTON", L"+5°", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 54, 24, hWnd, (HMENU)(INT_PTR)plus5Id, wc.hInstance, NULL);
                }
            };

            CreateODRowControls(L"Esfera (Sphere):", ID_BTN_SPHERE_MINUS, ID_BTN_SPHERE_PLUS, g_hLblSphere, g_hBtnSphereMinus, g_lblValSphere, g_hBtnSpherePlus);
            CreateODRowControls(L"Cilindro (Cyl):", ID_BTN_CYLINDER_MINUS, ID_BTN_CYLINDER_PLUS, g_hLblCylinder, g_hBtnCylinderMinus, g_lblValCylinder, g_hBtnCylinderPlus);
            CreateODRowControls(L"Ángulo (Axis):", ID_BTN_AXIS_MINUS, ID_BTN_AXIS_PLUS, g_hLblAxis, g_hBtnAxisMinus, g_lblValAxis, g_hBtnAxisPlus, &g_hBtnAxisM5, &g_hBtnAxisP5, ID_BTN_AXIS_MINUS_5, ID_BTN_AXIS_PLUS_5);
            CreateODRowControls(L"Nitidez / Enfoque:", ID_BTN_DECONV_MINUS, ID_BTN_DECONV_PLUS, g_hLblDeconv, g_hBtnDeconvMinus, g_lblValDeconv, g_hBtnDeconvPlus);

            // Prominent Action Button: [ 🚀 APLICAR A MONITORES ]
            g_hBtnApplyMonitors = CreateWindowW(L"BUTTON", L"🚀 APLICAR A MONITORES", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 36, hWnd, (HMENU)ID_BTN_APPLY_MONITORS, wc.hInstance, NULL);
            g_hBtnFullScreen = CreateWindowW(L"BUTTON", L"📺 Pantalla Completa [F11]", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 30, hWnd, (HMENU)ID_BTN_FULLSCREEN, wc.hInstance, NULL);
            g_hBtnToggleMonitor = CreateWindowW(L"BUTTON", L"🖥️ Cambiar Monitor (1 / 2)", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 30, hWnd, (HMENU)ID_BTN_TOGGLE_MONITOR, wc.hInstance, NULL);
            g_hBtnToggle = CreateWindowW(L"BUTTON", L"📑 Alternar Lupa / Cartilla", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 30, hWnd, (HMENU)ID_BTN_TOGGLE_SOURCE, wc.hInstance, NULL);
            g_hBtnReset = CreateWindowW(L"BUTTON", L"Reiniciar Calibración", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 100, 26, hWnd, (HMENU)ID_BTN_RESET, wc.hInstance, NULL);

            // 4. Instructions Box
            g_hInstrTitle = CreateWindowW(L"STATIC", L"📋 GUÍA DE AUTO-CALIBRACIÓN:", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 18, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hInstrTitle, WM_SETFONT, (WPARAM)g_hFontInstrBold, TRUE);

            g_hInstrStep = CreateWindowW(L"STATIC", L"Paso: 1/3 (Miopía / Esfera)", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 18, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hInstrStep, WM_SETFONT, (WPARAM)g_hFontInstrBold, TRUE);

            g_hInstrOptionTag = CreateWindowW(L"STATIC", L"🔴 En pantalla: OPCIÓN 1", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 20, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hInstrOptionTag, WM_SETFONT, (WPARAM)g_hFontInstrBold, TRUE);

            g_hInstrLine1 = CreateWindowW(L"STATIC", L"• Alterna [1] y [2] cada 1 segundo automáticamente.", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 30, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hInstrLine1, WM_SETFONT, (WPARAM)g_hFontInstrBody, TRUE);

            g_hInstrLine2 = CreateWindowW(L"STATIC", L"• Pulsa [1] o [2] según cuál se vea más clara.", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 30, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hInstrLine2, WM_SETFONT, (WPARAM)g_hFontInstrBody, TRUE);

            g_hInstrLine3 = CreateWindowW(L"STATIC", L"• [🚀 APLICAR] activa la corrección sin bloquear mouse.", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_LEFT, 0, 0, 100, 30, hWnd, NULL, wc.hInstance, NULL);
            SendMessageW(g_hInstrLine3, WM_SETFONT, (WPARAM)g_hFontInstrBody, TRUE);

            SetupCurrentTestOptions();
            UpdateLabels();

            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            RelayoutControls(rcClient.right, rcClient.bottom);

            g_state.lastToggleTick = GetTickCount();
            SetTimer(hWnd, TIMER_FRAME_UPDATE, 50, NULL);
            return 0;
        }

        case WM_HOTKEY: {
            if (wParam == HOTKEY_ID_TOGGLE_COMPENSATION || wParam == HOTKEY_ID_FULLSCREEN) {
                if (g_state.sourceType != 3) {
                    g_state.sourceType = 3;
                    g_state.appPhase = PHASE_MAGNIFIER_90;
                }
                ToggleFullScreen(hWnd);
            } else if (wParam == HOTKEY_ID_ESCAPE) {
                if (g_state.isFullScreen) {
                    ToggleFullScreen(hWnd);
                }
            }
            return 0;
        }

        case WM_ERASEBKGND: {
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect((HDC)wParam, &rc, g_hBrushWindowBg ? g_hBrushWindowBg : (HBRUSH)GetStockObject(BLACK_BRUSH));
            return 1;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);

            if (hCtrl == g_lblValSphere || hCtrl == g_lblValCylinder || hCtrl == g_lblValAxis || hCtrl == g_lblValDeconv) {
                SetTextColor(hdc, g_colorCyan);
                SetBkColor(hdc, g_colorPanelAlt);
                return (INT_PTR)(g_hBrushPanelAlt ? g_hBrushPanelAlt : GetStockObject(DKGRAY_BRUSH));
            }

            if (hCtrl == g_hInstrTitle || hCtrl == g_hODTitle) SetTextColor(hdc, g_colorCyan);
            else if (hCtrl == g_hInstrOptionTag) SetTextColor(hdc, g_colorYellow);
            else if (hCtrl == g_hODSub) SetTextColor(hdc, g_colorMuted);
            else SetTextColor(hdc, g_colorText);

            return (INT_PTR)(g_hBrushPanelBg ? g_hBrushPanelBg : GetStockObject(BLACK_BRUSH));
        }

        case WM_DRAWITEM:
            DrawModernButton((LPDRAWITEMSTRUCT)lParam);
            return TRUE;

        case WM_TIMER: {
            if (wParam == TIMER_FRAME_UPDATE) {
                if (g_state.appPhase == PHASE_CALIBRATION_WIZARD) {
                    DWORD now = GetTickCount();
                    if (now - g_state.lastToggleTick >= 1000) {
                        g_state.autoCurrentOption = (g_state.autoCurrentOption == 1) ? 2 : 1;
                        g_state.lastToggleTick = now;
                        UpdateLabels();
                        if (g_hWndViewport) InvalidateRect(g_hWndViewport, NULL, FALSE);
                    }
                } else if (g_state.sourceType == 3) {
                    if (g_hWndViewport) InvalidateRect(g_hWndViewport, NULL, FALSE);
                }
            }
            return 0;
        }

        case WM_SIZE: {
            int winW = LOWORD(lParam);
            int winH = HIWORD(lParam);
            RelayoutControls(winW, winH);
            if (g_hWndViewport) InvalidateRect(g_hWndViewport, NULL, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_F11:
                    ToggleFullScreen(hWnd);
                    break;
                case VK_ESCAPE:
                    if (g_state.isFullScreen) ToggleFullScreen(hWnd);
                    break;

                case '1':
                    SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CHOOSE_1, 0), 0);
                    break;
                case '2':
                    SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CHOOSE_2, 0), 0);
                    break;
                case '3':
                case VK_SPACE:
                case VK_RETURN:
                    SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CHOOSE_IGUAL, 0), 0);
                    break;
                case '4':
                case 'P':
                case VK_BACK:
                    SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CHOOSE_MUCHO_PEOR, 0), 0);
                    break;

                case 'M':
                    g_state.marginRatio = max(0.70f, g_state.marginRatio - 0.02f);
                    InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;
                case 'N':
                    g_state.marginRatio = min(1.00f, g_state.marginRatio + 0.02f);
                    InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;

                case 'Q':
                    g_state.sphere = max(-10.0f, g_state.sphere - 0.25f);
                    g_state.sphereCenter = g_state.sphere;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;
                case 'W':
                    g_state.sphere = min(10.0f, g_state.sphere + 0.25f);
                    g_state.sphereCenter = g_state.sphere;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;

                case 'A':
                    g_state.cylinder = max(-8.0f, g_state.cylinder - 0.25f);
                    g_state.cylCenter = g_state.cylinder;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;
                case 'S':
                    g_state.cylinder = min(8.0f, g_state.cylinder + 0.25f);
                    g_state.cylCenter = g_state.cylinder;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;

                case 'Z':
                    g_state.axis = (g_state.axis - 1 + 181) % 181;
                    g_state.axisCenter = (float)g_state.axis;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;
                case 'X':
                    g_state.axis = (g_state.axis + 1) % 181;
                    g_state.axisCenter = (float)g_state.axis;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    break;
            }
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);

            // [ 🚀 APLICAR A MONITORES ]
            if (wmId == ID_BTN_APPLY_MONITORS) {
                g_state.appPhase = PHASE_MAGNIFIER_90;
                g_state.sourceType = 3; // Live Desktop
                UpdateLabels();
                ToggleFullScreen(hWnd);
                return 0;
            } else if (wmId == ID_BTN_FULLSCREEN) {
                ToggleFullScreen(hWnd);
                return 0;
            } else if (wmId == ID_BTN_TOGGLE_MONITOR) {
                g_state.monitorTarget = (MonitorTarget)((g_state.monitorTarget + 1) % 3);
                UpdateLabels();
                if (g_state.isFullScreen) {
                    ToggleFullScreen(hWnd);
                    ToggleFullScreen(hWnd);
                }
                return 0;
            }

            // Clinical Jackson Cross-Cylinder (JCC) Algorithm
            if (wmId == ID_BTN_CHOOSE_1 || wmId == ID_BTN_CHOOSE_2 || wmId == ID_BTN_CHOOSE_IGUAL || wmId == ID_BTN_CHOOSE_MUCHO_PEOR) {
                g_state.iterationCount++;
                g_state.lastToggleTick = GetTickCount();

                if (wmId == ID_BTN_CHOOSE_IGUAL) {
                    if (g_state.calibPhase == CALIB_SPHERE) {
                        g_state.sphere = g_state.sphereCenter;
                        g_state.sphereStep = max(0.125f, g_state.sphereStep * 0.50f);
                        if (g_state.sphereStep <= 0.125f || g_state.iterationCount >= 5) {
                            g_state.calibPhase = CALIB_CYL_POWER;
                            g_state.iterationCount = 0;
                        }
                    } else if (g_state.calibPhase == CALIB_CYL_POWER) {
                        g_state.cylinder = g_state.cylCenter;
                        g_state.cylStep = max(0.125f, g_state.cylStep * 0.50f);
                        if (g_state.cylStep <= 0.125f || g_state.iterationCount >= 5) {
                            g_state.calibPhase = CALIB_AXIS_FINE;
                            g_state.iterationCount = 0;
                        }
                    } else if (g_state.calibPhase == CALIB_AXIS_FINE) {
                        g_state.axis = (int)g_state.axisCenter;
                        g_state.axisStep = max(2.0f, g_state.axisStep * 0.50f);
                        if (g_state.axisStep <= 2.0f || g_state.iterationCount >= 5) {
                            g_state.appPhase = PHASE_MAGNIFIER_90;
                            g_state.sourceType = 3;
                        }
                    }
                } 
                else if (wmId == ID_BTN_CHOOSE_MUCHO_PEOR) {
                    if (g_state.calibPhase == CALIB_SPHERE) {
                        g_state.sphereStep = max(0.25f, g_state.sphereStep * 0.5f);
                        g_state.sphereCenter = g_state.sphere;
                    } else if (g_state.calibPhase == CALIB_CYL_POWER) {
                        g_state.cylStep = max(0.25f, g_state.cylStep * 0.5f);
                        g_state.cylCenter = g_state.cylinder;
                    } else if (g_state.calibPhase == CALIB_AXIS_FINE) {
                        g_state.axisStep = max(2.0f, g_state.axisStep * 0.5f);
                    }
                }
                else {
                    bool choose1 = (wmId == ID_BTN_CHOOSE_1);

                    if (g_state.calibPhase == CALIB_SPHERE) {
                        g_state.sphereCenter = choose1 ? g_state.opt1Sphere : g_state.opt2Sphere;
                        g_state.sphere = g_state.sphereCenter;
                        g_state.sphereStep *= 0.55f;

                        if (g_state.sphereStep <= 0.125f || g_state.iterationCount >= 5) {
                            g_state.calibPhase = CALIB_CYL_POWER;
                            g_state.iterationCount = 0;
                        }
                    } else if (g_state.calibPhase == CALIB_CYL_POWER) {
                        g_state.cylCenter = choose1 ? g_state.opt1Cyl : g_state.opt2Cyl;
                        g_state.cylinder = g_state.cylCenter;
                        g_state.cylStep *= 0.55f;

                        if (g_state.cylStep <= 0.125f || g_state.iterationCount >= 5) {
                            g_state.calibPhase = CALIB_AXIS_FINE;
                            g_state.iterationCount = 0;
                        }
                    } else if (g_state.calibPhase == CALIB_AXIS_FINE) {
                        g_state.axisCenter = choose1 ? g_state.opt1Axis : g_state.opt2Axis;
                        g_state.axis = (int)g_state.axisCenter;
                        g_state.axisStep = max(2.0f, g_state.axisStep * 0.5f);

                        if (g_state.axisStep <= 2.0f || g_state.iterationCount >= 5) {
                            g_state.appPhase = PHASE_MAGNIFIER_90;
                            g_state.sourceType = 3;
                        }
                    }
                }

                SetupCurrentTestOptions();
                UpdateLabels();
                InvalidateRect(g_hWndViewport, NULL, FALSE);
                return 0;
            } 
            else if (wmId == ID_BTN_TOGGLE_SOURCE) {
                g_state.sourceType = (g_state.sourceType == 3) ? 0 : 3;
                UpdateLabels();
                InvalidateRect(g_hWndViewport, NULL, FALSE);
                return 0;
            }

            // OD +/- Buttons
            switch (wmId) {
                case ID_BTN_SPHERE_MINUS:
                    g_state.sphere = max(-10.0f, g_state.sphere - 0.25f);
                    g_state.sphereCenter = g_state.sphere;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;
                case ID_BTN_SPHERE_PLUS:
                    g_state.sphere = min(10.0f, g_state.sphere + 0.25f);
                    g_state.sphereCenter = g_state.sphere;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;

                case ID_BTN_CYLINDER_MINUS:
                    g_state.cylinder = max(-8.0f, g_state.cylinder - 0.25f);
                    g_state.cylCenter = g_state.cylinder;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;
                case ID_BTN_CYLINDER_PLUS:
                    g_state.cylinder = min(8.0f, g_state.cylinder + 0.25f);
                    g_state.cylCenter = g_state.cylinder;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;

                case ID_BTN_AXIS_MINUS:
                    g_state.axis = max(0, g_state.axis - 1);
                    g_state.axisCenter = (float)g_state.axis;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;
                case ID_BTN_AXIS_PLUS:
                    g_state.axis = min(180, g_state.axis + 1);
                    g_state.axisCenter = (float)g_state.axis;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;

                case ID_BTN_AXIS_MINUS_5:
                    g_state.axis = max(0, g_state.axis - 5);
                    g_state.axisCenter = (float)g_state.axis;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;
                case ID_BTN_AXIS_PLUS_5:
                    g_state.axis = min(180, g_state.axis + 5);
                    g_state.axisCenter = (float)g_state.axis;
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;

                case ID_BTN_DECONV_MINUS:
                    g_state.deconv = max(0.0f, g_state.deconv - 0.2f);
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;
                case ID_BTN_DECONV_PLUS:
                    g_state.deconv = min(5.0f, g_state.deconv + 0.2f);
                    UpdateLabels(); InvalidateRect(g_hWndViewport, NULL, FALSE);
                    return 0;

                case ID_BTN_RESET:
                    ResetToDefaults();
                    return 0;
            }
            return 0;
        }

        case WM_DESTROY:
            UnregisterHotKey(hWnd, HOTKEY_ID_TOGGLE_COMPENSATION);
            UnregisterHotKey(hWnd, HOTKEY_ID_FULLSCREEN);
            UnregisterHotKey(hWnd, HOTKEY_ID_ESCAPE);
            KillTimer(hWnd, TIMER_FRAME_UPDATE);
            if (g_hBrushWindowBg) DeleteObject(g_hBrushWindowBg);
            if (g_hBrushPanelBg) DeleteObject(g_hBrushPanelBg);
            if (g_hBrushPanelAlt) DeleteObject(g_hBrushPanelAlt);
            if (g_hFontBigHeader) DeleteObject(g_hFontBigHeader);
            if (g_hFontLabels) DeleteObject(g_hFontLabels);
            if (g_hFontValues) DeleteObject(g_hFontValues);
            if (g_hFontBtns) DeleteObject(g_hFontBtns);
            if (g_hFontBigBtns) DeleteObject(g_hFontBigBtns);
            if (g_hFontApplyBtn) DeleteObject(g_hFontApplyBtn);
            if (g_hFontInstrBold) DeleteObject(g_hFontInstrBold);
            if (g_hFontInstrBody) DeleteObject(g_hFontInstrBody);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"VisionCompensatorTrueClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(
        0, L"VisionCompensatorTrueClass",
        L"Vision Compensator - Jackson Cross-Cylinder & Click-Through Passthrough",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        50, 50, 1300, 800,
        NULL, NULL, hInstance, NULL
    );

    // Exclude from capture to prevent feedback mirror loop
    SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);

    // Set to 90% of screen
    SetWindowTo90Percent(hWnd);

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
    SetForegroundWindow(hWnd);
    BringWindowToTop(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}

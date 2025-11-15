#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <gdiplus.h>
#include <memory>
#include <sys/stat.h>
#include <windowsx.h>
#include <cstdarg>
#include <cstdlib>

// SQLite 支持
#define HAS_SQLITE 1
extern "C" {
#include "sqlite3.h"
}

#ifdef _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#endif

struct LinkRange {
    int startChar = 0;
    int length = 0;
    std::wstring url;
};

// 全局变量
HWND g_hwnd = NULL;
std::vector<std::wstring> g_lines;
std::vector<std::wstring> g_wrappedLines; // 换行后的行
std::vector<int> g_lineToOriginal; // 映射：换行后的行索引 -> 原始行索引
int g_scrollOffset = 0;
const int LINE_HEIGHT = 20;
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 400;
const int LEFT_MARGIN = 10;
const int RIGHT_MARGIN = 10;
const int TOP_MARGIN = 10;
const int DRAG_THRESHOLD = 3;
bool g_visible = true;
std::wstring g_currentFile;
std::wstring g_exeDirectory;
std::wstring g_historyDir = L"history";
std::wstring g_configFile = L"md_reader_config.ini";
std::wstring g_dbPath = L"history/history.db";
std::vector<std::vector<LinkRange>> g_lineLinks;
std::vector<std::vector<LinkRange>> g_wrappedLineLinks;
bool g_hoveredLinkActive = false;
int g_hoveredLinkDisplayIndex = -1;
LinkRange g_hoveredLinkRange;

enum TrayCommand {
    ID_TRAY_BG_SLIDER = 1001,
    ID_TRAY_TEXT_COLOR,
    ID_TRAY_TEXT_ALPHA,
    ID_TRAY_LINK_HIGHLIGHT,
    ID_TRAY_LOCK_WINDOW,
    ID_TRAY_EXIT,
    ID_TRAY_RELOAD,
    ID_TRAY_OPEN_FILE
};

COLORREF g_textColor = RGB(230, 230, 230);
COLORREF g_indicatorColor = RGB(120, 120, 180);

bool g_isDragging = false;
bool g_hasMovedDuringDrag = false;
POINT g_dragStart = {};
POINT g_windowStart = {};

// 关闭按钮相关
const int CLOSE_BUTTON_SIZE = 28;
const int CLOSE_BUTTON_MARGIN = 8;
RECT g_closeButtonRect = {};
bool g_closeButtonHovered = false;

// 翻页指示器相关
enum PageFlipDirection {
    FLIP_NONE = 0,
    FLIP_UP = 1,
    FLIP_DOWN = 2
};
const UINT_PTR TIMER_ID_PAGE_INDICATOR = 200;
const int PAGE_INDICATOR_DURATION = 300; // 毫秒
PageFlipDirection g_pageFlipDirection = FLIP_NONE;
POINT g_pageFlipClickPos = {};
DWORD g_pageFlipStartTime = 0;

// 锁定/穿透模式相关
bool g_isLocked = false; // false=交互模式(默认), true=穿透模式
const UINT_PTR TIMER_ID_CTRL_CHECK = 300;
const UINT_PTR TIMER_ID_MOUSE_CHECK = 301;
bool g_ctrlPressed = false; // Ctrl键是否按下
bool g_rightButtonPressed = false; // 右键是否按下
HWND g_prevForeground = NULL; // Ctrl按下时记录的前台窗口
const int LOCK_ICON_SIZE = 28;
const int LOCK_ICON_SPACING = 8; // 锁定图标与关闭按钮的间距
RECT g_lockIconRect = {};
bool g_lockIconHovered = false;

BYTE g_textAlpha = 255;
BYTE g_backgroundAlpha = 0; // 0 = 完全透明背景

// SQLite 历史数据库
sqlite3* g_historyDB = nullptr;
std::wstring g_dbPath = L"history/history.db";

Gdiplus::GdiplusStartupInput g_gdiplusInput;
ULONG_PTR g_gdiplusToken = 0;

NOTIFYICONDATA g_trayIcon = {};
constexpr UINT WM_TRAYICON = WM_APP + 1;
UINT g_taskbarCreatedMsg = 0;
bool g_trayIconAdded = false;
HICON g_customIcon = NULL;

HWND g_sliderWindow = NULL;
bool g_sliderForBackground = true;
int g_sliderCurrentValue = 0;
bool g_linkHighlightEnabled = true;

const int SLIDER_WIDTH = 220;
const int SLIDER_HEIGHT = 115;

#ifndef NDEBUG
void DebugLog(const wchar_t* format, ...)
{
    wchar_t buffer[256];
    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, _countof(buffer), format, args);
    va_end(args);
    OutputDebugStringW(buffer);
}
inline void DebugLog(const std::wstring& message)
{
    OutputDebugStringW(message.c_str());
}
#else
inline void DebugLog(const wchar_t* /*format*/, ...) {}
inline void DebugLog(const std::wstring& /*message*/) {}
#endif

struct SliderContext {
    bool forBackground = true;
    HWND trackbar = NULL;
    HWND valueLabel = NULL;
    HWND closeButton = NULL;
};

void RenderLayeredWindow();
void UpdateBackgroundAlpha(BYTE alpha);
void UpdateTextAlpha(BYTE alpha);
void ShowSliderWindow(bool forBackground);
void DestroySliderWindow();
void CreateOrUpdateTrayIcon(HINSTANCE hInstance);
void RemoveTrayIcon();
std::wstring CombinePath(const std::wstring& base, const std::wstring& name);
void InitializePaths();
void OpenFileDialog(HWND owner);
HMENU CreateTrayMenu();
LRESULT CALLBACK SliderWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void UpdateSliderLabelText(SliderContext* ctx, int value);
HICON CreateCustomTrayIcon();
void SaveProgress();
void LoadProgress();
UINT_PTR CALLBACK ColorDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
void WrapTextLines();
void ProcessMarkdownLinks();
std::unique_ptr<Gdiplus::Font> CreateRenderFont();
bool HitTestLink(int x, int y, std::wstring& url, int* outDisplayIndex = nullptr,
                 LinkRange* outSpan = nullptr);
bool TryHandleLinkClick(int x, int y);
std::wstring ResolveLinkTarget(const std::wstring& raw);
void LoadFileAndReset(const std::wstring& filepath);
bool ReadFileBinary(const std::wstring& filepath, std::string& outBuffer);
std::wstring Utf8ToWString(const std::string& utf8);
std::vector<std::wstring> LoadMarkdownFile(const std::wstring& filepath);
bool InitHistoryDB();
void CloseHistoryDB();
void SaveFileHistoryDB(const std::wstring& path);
bool LoadFileHistoryDB(const std::wstring& path);
void UpdateCloseButtonRect();
void DrawCloseButton(Gdiplus::Graphics& graphics, bool hovered);
bool IsPointInRect(int x, int y, const RECT& rect);
void ScrollHalfPage(bool scrollUp);
void DrawPageFlipIndicator(Gdiplus::Graphics& graphics);
void StartPageFlipAnimation(PageFlipDirection direction, int x, int y);
void ToggleLockState();
void UpdateLockIconRect();
void DrawLockIcon(Gdiplus::Graphics& graphics, bool hovered);
void UpdateClickThroughState();
void CheckCtrlKeyState();
void CheckMouseState();
void RestorePreviousForeground();

bool IsPointInRect(int x, int y, const RECT& rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void UpdateCloseButtonRect() {
    if (!g_hwnd) return;
    
    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);
    
    g_closeButtonRect.right = clientRect.right - CLOSE_BUTTON_MARGIN;
    g_closeButtonRect.left = g_closeButtonRect.right - CLOSE_BUTTON_SIZE;
    g_closeButtonRect.top = CLOSE_BUTTON_MARGIN;
    g_closeButtonRect.bottom = g_closeButtonRect.top + CLOSE_BUTTON_SIZE;
}

void UpdateLockIconRect() {
    if (!g_hwnd) return;
    
    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);
    
    g_lockIconRect.right = g_closeButtonRect.left - LOCK_ICON_SPACING;
    g_lockIconRect.left = g_lockIconRect.right - LOCK_ICON_SIZE;
    g_lockIconRect.top = CLOSE_BUTTON_MARGIN;
    g_lockIconRect.bottom = g_lockIconRect.top + LOCK_ICON_SIZE;
}

void DrawCloseButton(Gdiplus::Graphics& graphics, bool hovered) {
    float centerX = (g_closeButtonRect.left + g_closeButtonRect.right) / 2.0f;
    float centerY = (g_closeButtonRect.top + g_closeButtonRect.bottom) / 2.0f;
    float buttonSize = CLOSE_BUTTON_SIZE / 2.0f;
    
    BYTE alpha = g_textAlpha;
    BYTE r = GetRValue(g_textColor);
    BYTE g = GetGValue(g_textColor);
    BYTE b = GetBValue(g_textColor);
    
    if (hovered) {
        r = (std::min)(255, (int)r + (255 - r) * 20 / 100);
        g = (std::min)(255, (int)g + (255 - g) * 20 / 100);
        b = (std::min)(255, (int)b + (255 - b) * 20 / 100);
        
        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(alpha * 15 / 100, r, g, b));
        graphics.FillEllipse(&bgBrush, 
            g_closeButtonRect.left, g_closeButtonRect.top,
            CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE);
    }
    
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    
    Gdiplus::Pen pen(Gdiplus::Color(alpha, r, g, b), hovered ? 2.5f : 2.0f);
    pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);
    
    float crossSize = buttonSize * 0.45f;
    graphics.DrawLine(&pen,
        centerX - crossSize, centerY - crossSize,
        centerX + crossSize, centerY + crossSize);
    graphics.DrawLine(&pen,
        centerX + crossSize, centerY - crossSize,
        centerX - crossSize, centerY + crossSize);
    
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
}

void DrawLockIcon(Gdiplus::Graphics& graphics, bool hovered) {
    float centerX = (g_lockIconRect.left + g_lockIconRect.right) / 2.0f;
    float centerY = (g_lockIconRect.top + g_lockIconRect.bottom) / 2.0f;
    
    BYTE alpha = g_textAlpha;
    BYTE r = GetRValue(g_textColor);
    BYTE g = GetGValue(g_textColor);
    BYTE b = GetBValue(g_textColor);
    
    if (hovered) {
        r = (std::min)(255, (int)r + (255 - r) * 20 / 100);
        g = (std::min)(255, (int)g + (255 - g) * 20 / 100);
        b = (std::min)(255, (int)b + (255 - b) * 20 / 100);
        
        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(alpha * 15 / 100, r, g, b));
        graphics.FillEllipse(&bgBrush, 
            g_lockIconRect.left, g_lockIconRect.top,
            LOCK_ICON_SIZE, LOCK_ICON_SIZE);
    }
    
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    
    Gdiplus::Pen pen(Gdiplus::Color(alpha, r, g, b), hovered ? 2.5f : 2.0f);
    pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);
    
    float lockSize = LOCK_ICON_SIZE * 0.5f;
    float lockWidth = lockSize * 0.6f;
    float lockHeight = lockSize * 0.65f;
    float shackleHeight = lockSize * 0.35f;
    
    if (g_isLocked) {
        Gdiplus::SolidBrush lockBrush(Gdiplus::Color(alpha, r, g, b));
        graphics.FillRectangle(&lockBrush,
            centerX - lockWidth / 2, centerY,
            lockWidth, lockHeight);
        
        graphics.DrawArc(&pen,
            centerX - lockWidth / 2, centerY - shackleHeight,
            lockWidth, shackleHeight * 2,
            180, 180);
        
        Gdiplus::SolidBrush keyholeBrush(Gdiplus::Color(alpha, 
            GetRValue(g_textColor) > 128 ? 0 : 255,
            GetGValue(g_textColor) > 128 ? 0 : 255,
            GetBValue(g_textColor) > 128 ? 0 : 255));
        graphics.FillEllipse(&keyholeBrush,
            centerX - 2.0f, centerY + lockHeight * 0.3f - 2.0f,
            4.0f, 4.0f);
        
    } else {
        Gdiplus::SolidBrush lockBrush(Gdiplus::Color(alpha, r, g, b));
        graphics.FillRectangle(&lockBrush,
            centerX - lockWidth / 2, centerY,
            lockWidth, lockHeight);
        
        float openOffset = lockWidth * 0.5f;
        graphics.DrawArc(&pen,
            centerX - lockWidth / 2 + openOffset, centerY - shackleHeight,
            lockWidth, shackleHeight * 2,
            180, 180);
        
        Gdiplus::SolidBrush keyholeBrush(Gdiplus::Color(alpha, 
            GetRValue(g_textColor) > 128 ? 0 : 255,
            GetGValue(g_textColor) > 128 ? 0 : 255,
            GetBValue(g_textColor) > 128 ? 0 : 255));
        graphics.FillEllipse(&keyholeBrush,
            centerX - 2.0f, centerY + lockHeight * 0.3f - 2.0f,
            4.0f, 4.0f);
    }
    
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
}

void ToggleLockState() {
    g_isLocked = !g_isLocked;
    if (!g_isLocked) {
        g_prevForeground = NULL;
    }
    DebugLog(L"[ToggleLock] isLocked=%d\n", g_isLocked ? 1 : 0);
    UpdateClickThroughState();
    RenderLayeredWindow();
    SaveProgress();
}

void UpdateClickThroughState() {
    if (!g_hwnd) return;

    LONG exStyle = GetWindowLong(g_hwnd, GWL_EXSTYLE);

    bool shouldBeTransparent = g_isLocked && !g_ctrlPressed && !g_rightButtonPressed;

    if (shouldBeTransparent) {
        POINT cursor;
        if (GetCursorPos(&cursor)) {
            POINT pt = cursor;
            ScreenToClient(g_hwnd, &pt);
            if (IsPointInRect(pt.x, pt.y, g_closeButtonRect) ||
                IsPointInRect(pt.x, pt.y, g_lockIconRect)) {
                shouldBeTransparent = false;
                DebugLog(L"[UpdateClickThrough] mouseOnButton (%d,%d)\n", pt.x, pt.y);
            }
        }
    }

    if (shouldBeTransparent) {
        if (!(exStyle & WS_EX_TRANSPARENT)) {
            SetWindowLong(g_hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
            DebugLog(L"[UpdateClickThrough] add transparent\n");
        }
    } else {
        if (exStyle & WS_EX_TRANSPARENT) {
            SetWindowLong(g_hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
            DebugLog(L"[UpdateClickThrough] remove transparent\n");
        }
    }

    LONG newStyle = GetWindowLong(g_hwnd, GWL_EXSTYLE);
    DebugLog(L"[UpdateClickThrough] locked=%d ctrl=%d right=%d -> hasTransparent=%d\n",
             g_isLocked ? 1 : 0,
             g_ctrlPressed ? 1 : 0,
             g_rightButtonPressed ? 1 : 0,
             (newStyle & WS_EX_TRANSPARENT) ? 1 : 0);
}

void CheckCtrlKeyState() {
    bool ctrlNowPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

    if (ctrlNowPressed == g_ctrlPressed) {
        return;
    }

    DebugLog(L"[CheckCtrl] %d -> %d\n", g_ctrlPressed ? 1 : 0, ctrlNowPressed ? 1 : 0);

    if (ctrlNowPressed) {
        if (g_isLocked && !g_prevForeground) {
            HWND fg = GetForegroundWindow();
            if (fg && fg != g_hwnd) {
                g_prevForeground = fg;
            }
        }
    } else {
        if (g_isLocked) {
            RestorePreviousForeground();
        } else {
            g_prevForeground = NULL;
        }
    }

    g_ctrlPressed = ctrlNowPressed;

    if (g_isLocked) {
        UpdateClickThroughState();
        RenderLayeredWindow();
    }
}

void CheckMouseState() {
    if (!g_hwnd) return;

    bool rightNow = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    bool rightJustPressed = rightNow && !g_rightButtonPressed;
    bool rightJustReleased = !rightNow && g_rightButtonPressed;
    g_rightButtonPressed = rightNow;

    if (rightJustPressed) {
        DebugLog(L"[CheckMouse] rightDown\n");
    } else if (rightJustReleased) {
        DebugLog(L"[CheckMouse] rightUp\n");
    }

    if (g_isLocked) {
        UpdateClickThroughState();
    }

    if (rightJustPressed && g_isLocked) {
        POINT screenPt;
        if (GetCursorPos(&screenPt)) {
            POINT clientPt = screenPt;
            ScreenToClient(g_hwnd, &clientPt);

            RECT clientRect;
            GetClientRect(g_hwnd, &clientRect);
            int windowHeight = clientRect.bottom - clientRect.top;
            int midHeight = windowHeight / 2;

            bool scrollUp = clientPt.y < midHeight;
            DebugLog(L"[CheckMouse] pageFlip %s at (%d,%d)\n", scrollUp ? L"UP" : L"DOWN", clientPt.x, clientPt.y);
            ScrollHalfPage(scrollUp);
            StartPageFlipAnimation(scrollUp ? FLIP_UP : FLIP_DOWN, clientPt.x, clientPt.y);
        }
    }
}

void RestorePreviousForeground() {
    if (!g_prevForeground || g_prevForeground == g_hwnd) {
        g_prevForeground = NULL;
        return;
    }
    if (!IsWindow(g_prevForeground)) {
        g_prevForeground = NULL;
        return;
    }

    DWORD currentThreadId = GetCurrentThreadId();
    DWORD targetProcessId = 0;
    DWORD targetThreadId = GetWindowThreadProcessId(g_prevForeground, &targetProcessId);

    BOOL attached = FALSE;
    if (targetThreadId != 0 && targetThreadId != currentThreadId) {
        attached = AttachThreadInput(currentThreadId, targetThreadId, TRUE);
    }

    if (targetProcessId != 0) {
        AllowSetForegroundWindow(targetProcessId);
    }

    SetForegroundWindow(g_prevForeground);
    SetFocus(g_prevForeground);

    if (attached) {
        AttachThreadInput(currentThreadId, targetThreadId, FALSE);
    }

    g_prevForeground = NULL;
}

bool InitHistoryDB() {
    CreateDirectoryW(g_historyDir.c_str(), NULL);
    
    int len = WideCharToMultiByte(CP_UTF8, 0, g_dbPath.c_str(), -1, NULL, 0, NULL, NULL);
    std::string dbPath(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, g_dbPath.c_str(), -1, &dbPath[0], len, NULL, NULL);
    dbPath.resize(len - 1);
    
    if (sqlite3_open_v2(dbPath.c_str(), &g_historyDB, 
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) != SQLITE_OK) {
        if (g_historyDB) sqlite3_close(g_historyDB);
        g_historyDB = nullptr;
        return false;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS history(path TEXT PRIMARY KEY,scroll INT,locked INT);";
    char* err = nullptr;
    sqlite3_exec(g_historyDB, sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    
    return true;
}

void CloseHistoryDB() {
    if (g_historyDB) {
        sqlite3_close(g_historyDB);
        g_historyDB = nullptr;
    }
}

void SaveFileHistoryDB(const std::wstring& path) {
    if (!g_historyDB || path.empty()) return;
    
    wchar_t full[MAX_PATH];
    GetFullPathNameW(path.c_str(), MAX_PATH, full, NULL);
    std::wstring lower(full);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    
    int len = WideCharToMultiByte(CP_UTF8, 0, lower.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, lower.c_str(), -1, &utf8[0], len, NULL, NULL);
    utf8.resize(len - 1);
    
    const char* sql = "INSERT OR REPLACE INTO history(path,scroll,locked)VALUES(?,?,?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(g_historyDB, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, utf8.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, g_scrollOffset);
        sqlite3_bind_int(stmt, 3, g_isLocked ? 1 : 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

bool LoadFileHistoryDB(const std::wstring& path) {
    if (!g_historyDB || path.empty()) return false;
    
    wchar_t full[MAX_PATH];
    GetFullPathNameW(path.c_str(), MAX_PATH, full, NULL);
    std::wstring lower(full);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    
    int len = WideCharToMultiByte(CP_UTF8, 0, lower.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, lower.c_str(), -1, &utf8[0], len, NULL, NULL);
    utf8.resize(len - 1);
    
    const char* sql = "SELECT scroll,locked FROM history WHERE path=?;";
    sqlite3_stmt* stmt = nullptr;
    bool found = false;
    
    if (sqlite3_prepare_v2(g_historyDB, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, utf8.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            g_scrollOffset = sqlite3_column_int(stmt, 0);
            g_isLocked = (sqlite3_column_int(stmt, 1) != 0);
            found = true;
        }
        sqlite3_finalize(stmt);
    }
    
    return found;
}

void UpdateBackgroundAlpha(BYTE alpha) {
    g_backgroundAlpha = alpha;
    RenderLayeredWindow();
    SaveProgress();
}

void UpdateTextAlpha(BYTE alpha) {
    g_textAlpha = alpha;
    RenderLayeredWindow();
    SaveProgress();
}

void UpdateSliderLabelText(SliderContext* ctx, int value) {
    if (!ctx || !ctx->valueLabel) {
        return;
    }
    double percent = (static_cast<double>(value) / 255.0) * 100.0;
    wchar_t buffer[64];
    swprintf_s(buffer, L"当前: %d (%.0f%%)", value, percent);
    SetWindowText(ctx->valueLabel, buffer);
}

void DestroySliderWindow() {
    if (g_sliderWindow) {
        DestroyWindow(g_sliderWindow);
        g_sliderWindow = NULL;
    }
}

void ShowSliderWindow(bool forBackground) {
    if (g_sliderWindow) {
        SliderContext* ctx = reinterpret_cast<SliderContext*>(GetWindowLongPtr(g_sliderWindow, GWLP_USERDATA));
        if (ctx) {
            ctx->forBackground = forBackground;
            int initialValue = forBackground ? g_backgroundAlpha : g_textAlpha;
            SendMessage(ctx->trackbar, TBM_SETPOS, TRUE, initialValue);
            UpdateSliderLabelText(ctx, initialValue);
        }
        SetWindowText(g_sliderWindow, forBackground ? L"背景透明度" : L"文字透明度");
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        int x = cursorPos.x - SLIDER_WIDTH / 2;
        int y = cursorPos.y - SLIDER_HEIGHT / 2;
        SetWindowPos(g_sliderWindow, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
        SetForegroundWindow(g_sliderWindow);
        return;
    }

    static bool registered = false;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = SliderWindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"MdReaderSliderWindow";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        if (!RegisterClass(&wc)) {
            return;
        }
        registered = true;
    }

    SliderContext* context = new SliderContext();
    context->forBackground = forBackground;

    g_sliderWindow = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        L"MdReaderSliderWindow",
        forBackground ? L"背景透明度" : L"文字透明度",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, SLIDER_WIDTH, SLIDER_HEIGHT,
        g_hwnd, NULL, hInstance, context);

    if (!g_sliderWindow) {
        delete context;
        return;
    }

    POINT cursorPos;
    GetCursorPos(&cursorPos);
    int x = cursorPos.x - SLIDER_WIDTH / 2;
    int y = cursorPos.y - SLIDER_HEIGHT / 2;
    SetWindowPos(g_sliderWindow, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
    SetForegroundWindow(g_sliderWindow);
}

LRESULT CALLBACK SliderWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
        {
            auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SliderContext* ctx = reinterpret_cast<SliderContext*>(cs->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));

            ctx->trackbar = CreateWindowEx(
                0, TRACKBAR_CLASS, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                10, 20, SLIDER_WIDTH - 20, 30,
                hwnd, reinterpret_cast<HMENU>(1), cs->hInstance, NULL);

            SendMessage(ctx->trackbar, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
            SendMessage(ctx->trackbar, TBM_SETTICFREQ, 25, 0);

            ctx->valueLabel = CreateWindowEx(
                0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
                10, 55, SLIDER_WIDTH - 20, 20,
                hwnd, NULL, cs->hInstance, NULL);

            // 添加关闭按钮
            ctx->closeButton = CreateWindowEx(
                0, L"BUTTON", L"关闭", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                SLIDER_WIDTH / 2 - 40, 80, 80, 25,
                hwnd, reinterpret_cast<HMENU>(2), cs->hInstance, NULL);

            int initialValue = ctx->forBackground ? g_backgroundAlpha : g_textAlpha;
            SendMessage(ctx->trackbar, TBM_SETPOS, TRUE, initialValue);
            UpdateSliderLabelText(ctx, initialValue);
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == 2) { // 关闭按钮的ID是2
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        case WM_HSCROLL:
        {
            SliderContext* ctx = reinterpret_cast<SliderContext*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!ctx || reinterpret_cast<HWND>(lParam) != ctx->trackbar) {
                break;
            }
            int value = static_cast<int>(SendMessage(ctx->trackbar, TBM_GETPOS, 0, 0));
            if (ctx->forBackground) {
                UpdateBackgroundAlpha(static_cast<BYTE>(value));
            } else {
                UpdateTextAlpha(static_cast<BYTE>(value));
            }
            UpdateSliderLabelText(ctx, value);
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
        {
            SliderContext* ctx = reinterpret_cast<SliderContext*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (ctx) {
                delete ctx;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            }
            g_sliderWindow = NULL;
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

HMENU CreateTrayMenu() {
    HMENU menu = CreatePopupMenu();
    if (!menu) {
        return NULL;
    }

    AppendMenu(menu, MF_STRING, ID_TRAY_BG_SLIDER, L"背景透明度…");
    AppendMenu(menu, MF_STRING, ID_TRAY_TEXT_ALPHA, L"文字透明度…");
    AppendMenu(menu, MF_STRING | (g_linkHighlightEnabled ? MF_CHECKED : MF_UNCHECKED),
               ID_TRAY_LINK_HIGHLIGHT, L"悬停链接高亮");
    AppendMenu(menu, MF_STRING, ID_TRAY_TEXT_COLOR, L"文字颜色…");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    // 锁定窗口选项（带勾选标记）
    AppendMenu(menu, MF_STRING | (g_isLocked ? MF_CHECKED : MF_UNCHECKED), 
               ID_TRAY_LOCK_WINDOW, L"🔒 锁定窗口（穿透模式）");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(menu, MF_STRING, ID_TRAY_RELOAD, L"重新加载当前文件");
    AppendMenu(menu, MF_STRING, ID_TRAY_OPEN_FILE, L"打开文件…");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(menu, MF_STRING, ID_TRAY_EXIT, L"退出");

    return menu;
}

void CreateOrUpdateTrayIcon(HINSTANCE hInstance) {
    if (!g_hwnd) {
        return;
    }

    if (!g_customIcon) {
        g_customIcon = CreateCustomTrayIcon();
    }

    ZeroMemory(&g_trayIcon, sizeof(NOTIFYICONDATA));
    g_trayIcon.cbSize = sizeof(NOTIFYICONDATA);
    g_trayIcon.hWnd = g_hwnd;
    g_trayIcon.uID = 1;
    g_trayIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_trayIcon.uCallbackMessage = WM_TRAYICON;
    g_trayIcon.hIcon = g_customIcon ? g_customIcon : LoadIcon(NULL, IDI_APPLICATION);

    wcscpy_s(g_trayIcon.szTip, L"Transparent MD Reader");

    BOOL result = FALSE;
    if (g_trayIconAdded) {
        result = Shell_NotifyIcon(NIM_MODIFY, &g_trayIcon);
    } else {
        result = Shell_NotifyIcon(NIM_ADD, &g_trayIcon);
        if (result) {
            g_trayIconAdded = true;
        }
    }
}

void RemoveTrayIcon() {
    if (g_trayIconAdded) {
        Shell_NotifyIcon(NIM_DELETE, &g_trayIcon);
        g_trayIconAdded = false;
    }
    if (g_customIcon) {
        DestroyIcon(g_customIcon);
        g_customIcon = NULL;
    }
}

void OpenFileDialog(HWND owner) {
    wchar_t filePath[MAX_PATH] = L"";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"Markdown 文件 (*.md)\0*.md\0文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"md";

    if (GetOpenFileName(&ofn)) {
        LoadFileAndReset(filePath);
    }
}

// 全局变量用于保存原始颜色
COLORREF g_originalTextColor = RGB(230, 230, 230);
const UINT_PTR TIMER_ID_COLOR_PREVIEW = 100;

// 颜色对话框钩子回调函数，用于实时预览颜色
UINT_PTR CALLBACK ColorDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
    static HWND s_colorSample = NULL;
    static CHOOSECOLOR* s_chooseColor = nullptr;
    static COLORREF s_lastPreviewColor = RGB(0,0,0);

    switch (uiMsg) {
        case WM_INITDIALOG:
        {
            s_chooseColor = reinterpret_cast<CHOOSECOLOR*>(lParam);
            s_colorSample = NULL;
            for (HWND child = GetWindow(hdlg, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
                wchar_t className[32] = {};
                if (GetClassName(child, className, static_cast<int>(sizeof(className) / sizeof(className[0])))) {
                    if (_wcsicmp(className, L"STATIC") == 0) {
                        LONG_PTR style = GetWindowLongPtr(child, GWL_STYLE);
                        if (style & SS_OWNERDRAW) {
                            s_colorSample = child;
                            break;
                        }
                    }
                }
            }
            // 启动定时器，轮询RGB编辑框以获得实时颜色
            SetTimer(hdlg, TIMER_ID_COLOR_PREVIEW, 80, NULL);
            s_lastPreviewColor = g_textColor;
            return TRUE;
        }

        case WM_DRAWITEM:
        {
            if (lParam) {
                auto drawInfo = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
                if (drawInfo && s_colorSample && drawInfo->hwndItem == s_colorSample) {
                    COLORREF currentColor = static_cast<COLORREF>(drawInfo->itemData & 0x00FFFFFFu);
                    if (currentColor != g_textColor) {
                        g_textColor = currentColor;
                        if (g_hwnd) {
                            RenderLayeredWindow();
                        }
                    }
                }
            }
            break;
        }

        case WM_TIMER:
        {
            if (wParam != TIMER_ID_COLOR_PREVIEW) break;
            // 遍历对话框所有编辑框，收集可能的 RGB 数值（0..255）
            int rgbValues[3] = { -1, -1, -1 };
            int count = 0;
            for (HWND child = GetWindow(hdlg, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
                wchar_t className[16] = {};
                if (!GetClassName(child, className, 16)) continue;
                if (_wcsicmp(className, L"Edit") != 0) continue;

                wchar_t text[16] = {};
                GetWindowText(child, text, 16);
                // 跳过空文本
                if (text[0] == L'\0') continue;
                // 解析数字
                int value = _wtoi(text);
                if (value >= 0 && value <= 255) {
                    if (count < 3) {
                        rgbValues[count] = value;
                        ++count;
                    } else {
                        // 保留最近三个数值，通常RGB位于遍历的末尾
                        rgbValues[0] = rgbValues[1];
                        rgbValues[1] = rgbValues[2];
                        rgbValues[2] = value;
                    }
                }
            }

            if (count >= 3 && rgbValues[0] >= 0 && rgbValues[1] >= 0 && rgbValues[2] >= 0) {
                COLORREF current = RGB(rgbValues[0], rgbValues[1], rgbValues[2]);
                if (current != s_lastPreviewColor) {
                    s_lastPreviewColor = current;
                    if (current != g_textColor) {
                        g_textColor = current;
                        if (g_hwnd) {
                            RenderLayeredWindow();
                        }
                    }
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            UINT id = LOWORD(wParam);
            UINT code = HIWORD(wParam);
            HWND ctrl = reinterpret_cast<HWND>(lParam);
            if (ctrl) {
                wchar_t className[32] = {};
                GetClassName(ctrl, className, 32);
                DebugLog(L"WM_COMMAND id=" + std::to_wstring(id) + L" code=" + std::to_wstring(code) + L" class=" + className);
                if (code == EN_CHANGE) {
                    wchar_t buffer[16] = {};
                    GetWindowText(ctrl, buffer, 16);
                    DebugLog(L"  text=" + std::wstring(buffer));
                }
            } else {
                DebugLog(L"WM_COMMAND id=" + std::to_wstring(id) + L" code=" + std::to_wstring(code) + L" (no ctrl)");
            }

            if (code == EN_CHANGE && s_chooseColor) {
                int red = GetDlgItemInt(hdlg, 0x041C, NULL, FALSE);
                int green = GetDlgItemInt(hdlg, 0x041D, NULL, FALSE);
                int blue = GetDlgItemInt(hdlg, 0x041E, NULL, FALSE);
                DebugLog(L"RGB edits -> R=" + std::to_wstring(red) + L" G=" + std::to_wstring(green) + L" B=" + std::to_wstring(blue));
            }
            break;
        }

        case WM_DESTROY:
            KillTimer(hdlg, TIMER_ID_COLOR_PREVIEW);
            s_colorSample = NULL;
            s_chooseColor = nullptr;
            break;
    }

    return FALSE;
}

HICON CreateCustomTrayIcon() {
    const int iconSize = 16;
    HDC hdcScreen = GetDC(NULL);
    if (!hdcScreen) {
        return LoadIcon(NULL, IDI_APPLICATION);
    }

    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    if (!hdcMem) {
        ReleaseDC(NULL, hdcScreen);
        return LoadIcon(NULL, IDI_APPLICATION);
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, iconSize, iconSize);
    if (!hBitmap) {
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return LoadIcon(NULL, IDI_APPLICATION);
    }

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    Gdiplus::Graphics graphics(hdcMem);
    if (graphics.GetLastStatus() != Gdiplus::Ok) {
        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return LoadIcon(NULL, IDI_APPLICATION);
    }

    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    
    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 45, 45, 48));
    graphics.FillRectangle(&bgBrush, 0, 0, iconSize, iconSize);
    
    Gdiplus::Pen borderPen(Gdiplus::Color(255, 100, 100, 120), 1.0f);
    graphics.DrawRectangle(&borderPen, 0, 0, iconSize - 1, iconSize - 1);
    
    Gdiplus::Font font(L"Consolas", 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 180, 220, 255));
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    
    Gdiplus::RectF rect(0, 0, (Gdiplus::REAL)iconSize, (Gdiplus::REAL)iconSize);
    graphics.DrawString(L"M", -1, &font, rect, &format, &textBrush);

    SelectObject(hdcMem, hOldBitmap);

    ICONINFO iconInfo = {};
    iconInfo.fIcon = TRUE;
    iconInfo.hbmColor = hBitmap;
    iconInfo.hbmMask = CreateBitmap(iconSize, iconSize, 1, 1, NULL);
    
    HICON hIcon = CreateIconIndirect(&iconInfo);
    
    DeleteObject(iconInfo.hbmMask);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    if (!hIcon) {
        return LoadIcon(NULL, IDI_APPLICATION);
    }
    
    return hIcon;
}

static std::wstring CombinePath(const std::wstring& base, const std::wstring& name) {
    if (base.empty()) {
        return name;
    }
    wchar_t last = base.back();
    std::wstring combined = base;
    if (last != L'\\' && last != L'/') {
        combined.push_back(L'\\');
    }
    combined += name;
    return combined;
}

void InitializePaths() {
    wchar_t modulePath[MAX_PATH] = {};
    if (GetModuleFileNameW(NULL, modulePath, MAX_PATH) > 0) {
        std::wstring fullPath(modulePath);
        size_t sep = fullPath.find_last_of(L"\\/");
        if (sep != std::wstring::npos) {
            g_exeDirectory = fullPath.substr(0, sep);
        } else {
            g_exeDirectory = fullPath;
        }
    } else {
        g_exeDirectory = L".";
    }
    if (g_exeDirectory.empty()) {
        g_exeDirectory = L".";
    }
    g_configFile = CombinePath(g_exeDirectory, L"md_reader_config.ini");
    g_historyDir = CombinePath(g_exeDirectory, L"history");
    g_dbPath = CombinePath(g_historyDir, L"history.db");
    CreateDirectoryW(g_historyDir.c_str(), NULL);
}

void SaveProgress() {
    if (!g_currentFile.empty()) {
        SaveFileHistoryDB(g_currentFile);
    }
    
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, g_configFile.c_str(), L"w,ccs=UTF-8") != 0 || fp == nullptr) {
        return;
    }
    
    fwprintf(fp, L"[Config]\n");
    fwprintf(fp, L"LastFile=%ls\n", g_currentFile.c_str());
    fwprintf(fp, L"TextAlpha=%d\n", (int)g_textAlpha);
    fwprintf(fp, L"BackgroundAlpha=%d\n", (int)g_backgroundAlpha);
    fwprintf(fp, L"TextColor=%d\n", (int)g_textColor);
    fwprintf(fp, L"LinkHighlight=%d\n", g_linkHighlightEnabled ? 1 : 0);
    
    fclose(fp);
}

void LoadProgress() {
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, g_configFile.c_str(), L"r,ccs=UTF-8") != 0 || fp == nullptr) {
        return;
    }
    
    wchar_t line[1024];
    std::wstring lastFile;
    int scrollOffset = 0;
    int textAlpha = -1;
    int bgAlpha = -1;
    int textColor = -1;
    int isLocked = -1;
    int linkHighlight = -1;
    
    while (fgetws(line, 1024, fp)) {
        std::wstring wline(line);
        if (!wline.empty() && wline.back() == L'\n') {
            wline.pop_back();
        }
        if (!wline.empty() && wline.back() == L'\r') {
            wline.pop_back();
        }
        
        if (wline.find(L"LastFile=") == 0) {
            lastFile = wline.substr(9);
        } else if (wline.find(L"ScrollOffset=") == 0) {
            scrollOffset = _wtoi(wline.substr(13).c_str());
        } else if (wline.find(L"TextAlpha=") == 0) {
            textAlpha = _wtoi(wline.substr(10).c_str());
        } else if (wline.find(L"BackgroundAlpha=") == 0) {
            bgAlpha = _wtoi(wline.substr(16).c_str());
        } else if (wline.find(L"TextColor=") == 0) {
            textColor = _wtoi(wline.substr(10).c_str());
        } else if (wline.find(L"IsLocked=") == 0) {
            isLocked = _wtoi(wline.substr(9).c_str());
        } else if (wline.find(L"LinkHighlight=") == 0) {
            linkHighlight = _wtoi(wline.substr(14).c_str());
        }
    }
    
    fclose(fp);
    
    if (textAlpha >= 0 && textAlpha <= 255) {
        g_textAlpha = static_cast<BYTE>(textAlpha);
    }
    if (bgAlpha >= 0 && bgAlpha <= 255) {
        g_backgroundAlpha = static_cast<BYTE>(bgAlpha);
    }
    if (textColor >= 0) {
        g_textColor = static_cast<COLORREF>(textColor);
    }
    if (isLocked >= 0) {
        g_isLocked = (isLocked != 0);
    }
    if (linkHighlight >= 0) {
        g_linkHighlightEnabled = (linkHighlight != 0);
    }
    
    if (!lastFile.empty()) {
        FILE* testFile = nullptr;
        if (_wfopen_s(&testFile, lastFile.c_str(), L"r") == 0 && testFile != nullptr) {
            fclose(testFile);
            g_currentFile = lastFile;
            g_lines = LoadMarkdownFile(lastFile);
            ProcessMarkdownLinks();
            
            if (!LoadFileHistoryDB(lastFile)) {
                if (scrollOffset >= 0 && scrollOffset < (int)g_lines.size()) {
                    g_scrollOffset = scrollOffset;
                } else {
                    g_scrollOffset = 0;
                }
                if (isLocked >= 0) {
                    g_isLocked = (isLocked != 0);
                }
            }
        }
    }
}

bool ReadFileBinary(const std::wstring& filepath, std::string& outBuffer) {
    outBuffer.clear();
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, filepath.c_str(), L"rb") != 0 || fp == nullptr) {
        return false;
    }

    char temp[4096];
    size_t read = 0;
    while ((read = fread(temp, 1, sizeof(temp), fp)) > 0) {
        outBuffer.append(temp, read);
    }
    fclose(fp);
    return true;
}

std::wstring Utf8ToWString(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }

    int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (wideLength <= 0) {
        return std::wstring();
    }

    std::wstring wide(wideLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], wideLength);
    return wide;
}

std::vector<std::wstring> LoadMarkdownFile(const std::wstring& filepath) {
    std::vector<std::wstring> lines;
    std::string buffer;
    if (!ReadFileBinary(filepath, buffer)) {
        lines.push_back(L"无法打开文件: " + filepath);
        return lines;
    }

    if (buffer.size() >= 3 && (unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF) {
        buffer.erase(0, 3);
    }

    std::wstring content = Utf8ToWString(buffer);
    if (content.empty() && !buffer.empty()) {
        lines.push_back(L"无法解析为 UTF-8 编码");
        return lines;
    }

    size_t pos = 0;
    while (pos <= content.size()) {
        size_t end = content.find(L'\n', pos);
        std::wstring line;
        if (end == std::wstring::npos) {
            line = content.substr(pos);
            pos = content.size() + 1;
        } else {
            line = content.substr(pos, end - pos);
            pos = end + 1;
        }

        if (!line.empty() && line.back() == L'\r') {
            line.pop_back();
        }

        lines.push_back(line);
    }

    if (lines.empty()) {
        lines.push_back(L"文件为空");
    }

    return lines;
}

static bool RangeIntersectsExisting(int start, int length, const std::vector<LinkRange>& ranges) {
    int end = start + length;
    for (const auto& item : ranges) {
        int otherStart = item.startChar;
        int otherEnd = item.startChar + item.length;
        if (start < otherEnd && end > otherStart) {
            return true;
        }
    }
    return false;
}

static bool IsUrlTerminator(wchar_t ch) {
    if (iswspace(ch)) {
        return true;
    }
    switch (ch) {
        case L')':
        case L']':
        case L'>':
        case L'"':
            return true;
        default:
            return false;
    }
}

static bool IsTrailingPunctuation(wchar_t ch) {
    switch (ch) {
        case L'.':
        case L',':
        case L';':
        case L':':
        case L'!':
        case L'?':
            return true;
        default:
            return false;
    }
}

static std::wstring TrimLinkTargetText(const std::wstring& input) {
    if (input.empty()) return input;
    size_t start = 0;
    size_t end = input.size();
    while (start < end && (iswspace(input[start]) || input[start] == L'<' || input[start] == L'"')) {
        ++start;
    }
    while (end > start && (iswspace(input[end - 1]) || input[end - 1] == L'>' || input[end - 1] == L'"')) {
        --end;
    }
    return input.substr(start, end - start);
}

static void AppendAutoLinkRanges(const std::wstring& text, std::vector<LinkRange>& ranges) {
    size_t searchPos = 0;
    while (searchPos < text.size()) {
        size_t candidate = std::wstring::npos;
        auto updateCandidate = [&](const std::wstring& prefix) {
            size_t pos = text.find(prefix, searchPos);
            if (pos != std::wstring::npos) {
                if (candidate == std::wstring::npos || pos < candidate) {
                    candidate = pos;
                }
            }
        };
        updateCandidate(L"http://");
        updateCandidate(L"https://");
        if (candidate == std::wstring::npos) {
            break;
        }
        
        size_t end = candidate;
        while (end < text.size() && !IsUrlTerminator(text[end])) {
            ++end;
        }
        while (end > candidate && IsTrailingPunctuation(text[end - 1])) {
            --end;
        }
        if (end <= candidate) {
            searchPos = candidate + 1;
            continue;
        }
        int startChar = static_cast<int>(candidate);
        int length = static_cast<int>(end - candidate);
        if (!RangeIntersectsExisting(startChar, length, ranges)) {
            LinkRange range;
            range.startChar = startChar;
            range.length = length;
            range.url = text.substr(candidate, end - candidate);
            ranges.push_back(std::move(range));
        }
        searchPos = end;
    }
}

void ProcessMarkdownLinks() {
    g_lineLinks.assign(g_lines.size(), {});
    for (size_t idx = 0; idx < g_lines.size(); ++idx) {
        const std::wstring& line = g_lines[idx];
        std::wstring sanitized;
        sanitized.reserve(line.size());
        std::vector<LinkRange> ranges;
        
        size_t pos = 0;
        auto tryParseLink = [&](size_t bracketPos, bool treatAsImage) -> bool {
            size_t closingBracket = line.find(L']', bracketPos + 1);
            if (closingBracket == std::wstring::npos) {
                return false;
            }
            if (closingBracket + 1 >= line.size() || line[closingBracket + 1] != L'(') {
                return false;
            }
            size_t closingParen = line.find(L')', closingBracket + 2);
            if (closingParen == std::wstring::npos) {
                return false;
            }
            std::wstring linkText = line.substr(bracketPos + 1, closingBracket - bracketPos - 1);
            std::wstring displayText = linkText;
            if (treatAsImage && displayText.empty()) {
                displayText = L"[image]";
            }
            std::wstring target = line.substr(closingBracket + 2, closingParen - (closingBracket + 2));
            target = TrimLinkTargetText(target);
            if (displayText.empty() || target.empty()) {
                return false;
            }
            int startChar = static_cast<int>(sanitized.size());
            sanitized += displayText;
            LinkRange range;
            range.startChar = startChar;
            range.length = static_cast<int>(displayText.size());
            range.url = target;
            ranges.push_back(std::move(range));
            pos = closingParen + 1;
            return true;
        };
        while (pos < line.size()) {
            if (line[pos] == L'\\' && pos + 1 < line.size()) {
                sanitized.push_back(line[pos + 1]);
                pos += 2;
                continue;
            }
            
            if (line[pos] == L'!' && pos + 1 < line.size() && line[pos + 1] == L'[') {
                if (tryParseLink(pos + 1, true)) {
                    continue;
                }
            }
            
            if (line[pos] == L'[') {
                if (tryParseLink(pos, false)) {
                    continue;
                }
            }
            
            sanitized.push_back(line[pos]);
            ++pos;
        }
        
        AppendAutoLinkRanges(sanitized, ranges);
        
        g_lines[idx] = sanitized;
        g_lineLinks[idx] = std::move(ranges);
    }
}

void LoadFileAndReset(const std::wstring& filepath) {
    g_lines = LoadMarkdownFile(filepath);
    ProcessMarkdownLinks();
    g_currentFile = filepath;
    
    if (!LoadFileHistoryDB(filepath)) {
        g_scrollOffset = 0;
        g_isLocked = false;
    }
    
    WrapTextLines();
    UpdateClickThroughState();
    if (g_hwnd) {
        RenderLayeredWindow();
    }
    SaveProgress();
}

std::unique_ptr<Gdiplus::Font> CreateRenderFont() {
    auto font = std::make_unique<Gdiplus::Font>(
        L"Consolas", 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    if (font->GetLastStatus() != Gdiplus::Ok) {
        font.reset(new Gdiplus::Font(
            L"Courier New", 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel));
    }
    if (font->GetLastStatus() != Gdiplus::Ok) {
        font.reset(new Gdiplus::Font(
            L"Arial", 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel));
    }
    if (font->GetLastStatus() != Gdiplus::Ok) {
        font.reset();
    }
    return font;
}

void WrapTextLines() {
    g_wrappedLines.clear();
    g_lineToOriginal.clear();
    g_wrappedLineLinks.clear();
    
    if (g_lines.empty() || !g_hwnd) {
        return;
    }
    
    RECT rect;
    GetClientRect(g_hwnd, &rect);
    int windowWidth = rect.right - rect.left;
    int textWidth = windowWidth - LEFT_MARGIN - RIGHT_MARGIN;
    
    auto font = CreateRenderFont();
    if (!font) {
        return;
    }
    
    if (textWidth <= 0) {
        for (size_t i = 0; i < g_lines.size(); ++i) {
            g_wrappedLines.push_back(g_lines[i]);
            g_lineToOriginal.push_back(static_cast<int>(i));
            g_wrappedLineLinks.emplace_back();
        }
        return;
    }
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 1, 1);
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
    
    Gdiplus::Graphics graphics(hdcMem);
    
    auto appendSegment = [&](size_t originalIdx, size_t startOffset, const std::wstring& text) {
        g_wrappedLines.push_back(text);
        g_lineToOriginal.push_back(static_cast<int>(originalIdx));
        std::vector<LinkRange> spans;
        if (originalIdx < g_lineLinks.size()) {
            size_t segmentEnd = startOffset + text.size();
            for (const auto& link : g_lineLinks[originalIdx]) {
                size_t linkStart = static_cast<size_t>(link.startChar);
                size_t linkEnd = linkStart + static_cast<size_t>(link.length);
                size_t overlapStart = std::max(startOffset, linkStart);
                size_t overlapEnd = std::min(segmentEnd, linkEnd);
                if (overlapStart < overlapEnd) {
                    LinkRange span;
                    span.startChar = static_cast<int>(overlapStart - startOffset);
                    span.length = static_cast<int>(overlapEnd - overlapStart);
                    span.url = link.url;
                    spans.push_back(std::move(span));
                }
            }
        }
        g_wrappedLineLinks.push_back(std::move(spans));
    };
    
    for (size_t lineIdx = 0; lineIdx < g_lines.size(); ++lineIdx) {
        const std::wstring& line = g_lines[lineIdx];
        
        if (line.empty()) {
            appendSegment(lineIdx, 0, L"");
            continue;
        }
        
        Gdiplus::RectF boundingBox;
        graphics.MeasureString(line.c_str(), -1, font.get(),
                             Gdiplus::PointF(0, 0), &boundingBox);
        
        if (boundingBox.Width <= textWidth) {
            appendSegment(lineIdx, 0, line);
            continue;
        }
        
        std::wstring currentLine;
        size_t segmentStart = 0;
        for (size_t i = 0; i < line.length(); ++i) {
            std::wstring testLine = currentLine + line[i];
            
            Gdiplus::RectF testBox;
            graphics.MeasureString(testLine.c_str(), -1, font.get(),
                                 Gdiplus::PointF(0, 0), &testBox);
            
            if (testBox.Width > textWidth && !currentLine.empty()) {
                appendSegment(lineIdx, segmentStart, currentLine);
                currentLine.assign(1, line[i]);
                segmentStart = i;
            } else {
                currentLine = std::move(testLine);
            }
        }
        
        if (!currentLine.empty()) {
            appendSegment(lineIdx, segmentStart, currentLine);
        }
    }
    
    SelectObject(hdcMem, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

static float MeasureTextWidth(const std::wstring& text, int charCount,
                              Gdiplus::Graphics& graphics, Gdiplus::Font* font) {
    if (charCount <= 0) {
        return 0.0f;
    }
    if (charCount > static_cast<int>(text.size())) {
        charCount = static_cast<int>(text.size());
    }
    if (charCount <= 0) return 0.0f;
    Gdiplus::RectF box;
    graphics.MeasureString(text.c_str(), charCount, font,
                           Gdiplus::PointF(0, 0), &box);
    return box.Width;
}

std::wstring ResolveLinkTarget(const std::wstring& raw) {
    std::wstring target = TrimLinkTargetText(raw);
    if (target.empty()) {
        return target;
    }
    if (target[0] == L'#') {
        return std::wstring();
    }
    bool hasProtocol = target.find(L"://") != std::wstring::npos;
    bool hasDrive = target.size() > 1 && target[1] == L':';
    bool isUNC = target.size() > 1 && target[0] == L'\\' && target[1] == L'\\';
    if (!hasProtocol && target.rfind(L"www.", 0) == 0) {
        target = L"https://" + target;
        hasProtocol = true;
    }
    if (hasProtocol || hasDrive || isUNC) {
        return target;
    }
    if (!g_currentFile.empty()) {
        std::wstring base = g_currentFile;
        size_t pos = base.find_last_of(L"\\/");
        std::wstring directory = (pos == std::wstring::npos) ? L"" : base.substr(0, pos + 1);
        std::wstring combined = directory + target;
        wchar_t buffer[MAX_PATH];
        DWORD len = GetFullPathNameW(combined.c_str(), MAX_PATH, buffer, NULL);
        if (len > 0 && len < MAX_PATH) {
            return std::wstring(buffer, len);
        }
        return combined;
    }
    return target;
}

bool HitTestLink(int x, int y, std::wstring& url, int* outDisplayIndex, LinkRange* outSpan) {
    if (!g_hwnd) return false;
    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);

    if (x < LEFT_MARGIN || x > clientRect.right - RIGHT_MARGIN) {
        return false;
    }
    int relativeY = y - TOP_MARGIN;
    if (relativeY < 0) {
        return false;
    }
    int lineInView = relativeY / LINE_HEIGHT;
    if (lineInView < 0) return false;

    const std::vector<std::wstring>& displayLines = g_wrappedLines.empty() ? g_lines : g_wrappedLines;
    int displayIndex = g_scrollOffset + lineInView;
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displayLines.size())) {
        return false;
    }

    const std::vector<std::vector<LinkRange>>* linkSource =
        g_wrappedLines.empty() ? &g_lineLinks : &g_wrappedLineLinks;
    if (displayIndex < 0 || displayIndex >= static_cast<int>(linkSource->size())) {
        return false;
    }
    const auto& spans = (*linkSource)[displayIndex];
    if (spans.empty()) {
        return false;
    }

    auto font = CreateRenderFont();
    if (!font) {
        return false;
    }

    HDC hdcScreen = GetDC(NULL);
    if (!hdcScreen) return false;
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    if (!hdcMem) {
        ReleaseDC(NULL, hdcScreen);
        return false;
    }
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 1, 1);
    if (!hBitmap) {
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return false;
    }
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

    Gdiplus::Graphics graphics(hdcMem);
    bool handled = false;
    float relativeX = static_cast<float>(x - LEFT_MARGIN);
    const std::wstring& lineText = displayLines[displayIndex];

    for (const auto& span : spans) {
        float startX = MeasureTextWidth(lineText, span.startChar, graphics, font.get());
        float endX = MeasureTextWidth(lineText, span.startChar + span.length, graphics, font.get());
        if (relativeX >= startX && relativeX <= endX) {
            url = span.url;
            if (outDisplayIndex) {
                *outDisplayIndex = displayIndex;
            }
            if (outSpan) {
                *outSpan = span;
            }
            handled = true;
            break;
        }
    }

    SelectObject(hdcMem, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return handled;
}

bool TryHandleLinkClick(int x, int y) {
    std::wstring link;
    if (!HitTestLink(x, y, link, nullptr, nullptr)) {
        return false;
    }
    std::wstring target = ResolveLinkTarget(link);
    if (target.empty()) {
        return false;
    }
    HINSTANCE result = ShellExecuteW(NULL, L"open", target.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)result <= 32) {
        std::wstring message = L"无法打开链接：\n" + target;
        MessageBox(g_hwnd, message.c_str(), L"提示", MB_OK | MB_ICONWARNING);
    }
    return true;
}

// 使用 GDI+ 绘制并更新分层窗口
void RenderLayeredWindow() {
    if (!g_hwnd) {
        return;
    }

    RECT rect = {};
    GetClientRect(g_hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    if (!hBitmap) {
        return;
    }

    HDC hdcMem = CreateCompatibleDC(NULL);
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

    Gdiplus::Graphics graphics(hdcMem);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(g_backgroundAlpha, 0, 0, 0));
    graphics.FillRectangle(&bgBrush, 0, 0, width, height);

    auto font = CreateRenderFont();
    if (!font) {
        SelectObject(hdcMem, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        return;
    }

    Gdiplus::Color baseTextColor(g_textAlpha,
        GetRValue(g_textColor), GetGValue(g_textColor), GetBValue(g_textColor));
    Gdiplus::SolidBrush textBrush(baseTextColor);
    Gdiplus::Color highlightColor(60,
        GetRValue(g_indicatorColor), GetGValue(g_indicatorColor), GetBValue(g_indicatorColor));
    Gdiplus::SolidBrush highlightBrush(highlightColor);
    Gdiplus::Pen underlinePen(baseTextColor, 1.0f);

    // 使用换行后的行数据
    const std::vector<std::wstring>& displayLines = g_wrappedLines.empty() ? g_lines : g_wrappedLines;
    const std::vector<std::vector<LinkRange>>& linkLines =
        g_wrappedLines.empty() ? g_lineLinks : g_wrappedLineLinks;
    
    int maxVisibleLines = (height - TOP_MARGIN) / LINE_HEIGHT;
    if (maxVisibleLines < 1) {
        maxVisibleLines = 1;
    }
    int startLine = g_scrollOffset;
    int endLine = std::min((int)displayLines.size(), startLine + maxVisibleLines);

    Gdiplus::REAL fontHeight = font->GetHeight(&graphics);
    float y = static_cast<float>(TOP_MARGIN);
    for (int i = startLine; i < endLine; ++i) {
        float lineY = y;
        const std::wstring& lineText = displayLines[i];
        const std::vector<LinkRange>* spans = nullptr;
        if (i >= 0 && i < static_cast<int>(linkLines.size())) {
            spans = &linkLines[i];
        }
        if (spans && !spans->empty()) {
            float highlightTop = lineY - 2.0f;
            float highlightHeight = static_cast<float>(LINE_HEIGHT) - 4.0f;
            for (const auto& span : *spans) {
                bool isHovered = g_hoveredLinkActive &&
                    g_hoveredLinkDisplayIndex == i &&
                    g_hoveredLinkRange.startChar == span.startChar &&
                    g_hoveredLinkRange.length == span.length;
                float startX = MeasureTextWidth(lineText, span.startChar, graphics, font.get());
                float endX = MeasureTextWidth(lineText, span.startChar + span.length, graphics, font.get());
                if (isHovered && g_linkHighlightEnabled && endX > startX) {
                    graphics.FillRectangle(&highlightBrush,
                        static_cast<Gdiplus::REAL>(LEFT_MARGIN) + startX, highlightTop,
                        endX - startX, highlightHeight);
                }
            }
        }
        graphics.DrawString(lineText.c_str(), -1, font.get(),
                            Gdiplus::PointF(static_cast<Gdiplus::REAL>(LEFT_MARGIN), lineY), &textBrush);
        if (spans && !spans->empty()) {
            float underlineY = lineY + fontHeight - 2.0f;
            for (const auto& span : *spans) {
                float startX = MeasureTextWidth(lineText, span.startChar, graphics, font.get());
                float endX = MeasureTextWidth(lineText, span.startChar + span.length, graphics, font.get());
                graphics.DrawLine(&underlinePen,
                    static_cast<Gdiplus::REAL>(LEFT_MARGIN) + startX, underlineY,
                    static_cast<Gdiplus::REAL>(LEFT_MARGIN) + endX, underlineY);
            }
        }
        y += static_cast<float>(LINE_HEIGHT);
    }

    if (displayLines.size() > static_cast<size_t>(maxVisibleLines)) {
        wchar_t info[64];
        swprintf_s(info, L"[%d/%d]", startLine + 1, (int)displayLines.size());
        Gdiplus::SolidBrush indicatorBrush(Gdiplus::Color(g_textAlpha,
            GetRValue(g_indicatorColor), GetGValue(g_indicatorColor), GetBValue(g_indicatorColor)));
        graphics.DrawString(info, -1, font.get(),
            Gdiplus::PointF(static_cast<Gdiplus::REAL>(width - 80), static_cast<Gdiplus::REAL>(height - 30)),
            &indicatorBrush);
    }
    
    // 绘制关闭按钮
    UpdateCloseButtonRect();
    DrawCloseButton(graphics, g_closeButtonHovered);
    
    // 绘制锁定图标
    UpdateLockIconRect();
    DrawLockIcon(graphics, g_lockIconHovered);
    
    // 绘制翻页指示器
    DrawPageFlipIndicator(graphics);

    BYTE* pixels = reinterpret_cast<BYTE*>(pvBits);
    size_t totalPixels = static_cast<size_t>(width) * static_cast<size_t>(height);
    for (size_t i = 0; i < totalPixels; ++i) {
        BYTE* px = pixels + i * 4;
        if (px[3] == 0) {
            px[3] = 1;
        }
    }

    POINT ptSrc = { 0, 0 };
    POINT ptDest;
    RECT windowRect;
    GetWindowRect(g_hwnd, &windowRect);
    ptDest.x = windowRect.left;
    ptDest.y = windowRect.top;

    SIZE size = { width, height };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(g_hwnd, NULL, &ptDest, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
}

// 滚动控制
void Scroll(int delta) {
    int oldOffset = g_scrollOffset;
    g_scrollOffset += delta;
    
    RECT rect;
    GetClientRect(g_hwnd, &rect);
    int maxVisibleLines = (rect.bottom - TOP_MARGIN) / LINE_HEIGHT;
    if (maxVisibleLines < 1) {
        maxVisibleLines = 1;
    }
    
    // 使用换行后的行数
    const std::vector<std::wstring>& displayLines = g_wrappedLines.empty() ? g_lines : g_wrappedLines;
    int maxScroll = std::max(0, (int)displayLines.size() - maxVisibleLines);
    
    if (g_scrollOffset < 0) g_scrollOffset = 0;
    if (g_scrollOffset > maxScroll) g_scrollOffset = maxScroll;
    
    RenderLayeredWindow();
    
    // 只有当滚动位置真的改变时才保存
    if (oldOffset != g_scrollOffset) {
        SaveProgress();
    }
}

// 半页滚动（向上或向下）
void ScrollHalfPage(bool scrollUp) {
    if (!g_hwnd) return;
    
    RECT rect;
    GetClientRect(g_hwnd, &rect);
    int maxVisibleLines = (rect.bottom - TOP_MARGIN) / LINE_HEIGHT;
    if (maxVisibleLines < 1) {
        maxVisibleLines = 1;
    }
    
    // 半页行数
    int halfPageLines = std::max(1, maxVisibleLines / 2);
    
    // 向上翻页是负数，向下翻页是正数
    int delta = scrollUp ? -halfPageLines : halfPageLines;
    Scroll(delta);
}

// 启动翻页动画
void StartPageFlipAnimation(PageFlipDirection direction, int x, int y) {
    g_pageFlipDirection = direction;
    g_pageFlipClickPos.x = x;
    g_pageFlipClickPos.y = y;
    g_pageFlipStartTime = GetTickCount();
    
    // 启动定时器用于淡出动画和自动清除
    SetTimer(g_hwnd, TIMER_ID_PAGE_INDICATOR, 50, NULL);
    
    // 立即重绘显示指示器
    RenderLayeredWindow();
}

// 绘制翻页指示器（箭头）
void DrawPageFlipIndicator(Gdiplus::Graphics& graphics) {
    if (g_pageFlipDirection == FLIP_NONE) return;
    
    // 计算经过的时间和透明度
    DWORD elapsed = GetTickCount() - g_pageFlipStartTime;
    if (elapsed > PAGE_INDICATOR_DURATION) {
        g_pageFlipDirection = FLIP_NONE;
        KillTimer(g_hwnd, TIMER_ID_PAGE_INDICATOR);
        return;
    }
    
    // 透明度从100%淡出到0%
    float alphaFactor = 1.0f - (static_cast<float>(elapsed) / PAGE_INDICATOR_DURATION);
    BYTE indicatorAlpha = static_cast<BYTE>(g_textAlpha * alphaFactor);
    
    if (indicatorAlpha < 10) return; // 太透明就不绘制了
    
    // 使用文字颜色
    BYTE r = GetRValue(g_textColor);
    BYTE g = GetGValue(g_textColor);
    BYTE b = GetBValue(g_textColor);
    
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    
    // 箭头大小和位置
    float arrowSize = 40.0f;
    float centerX = static_cast<float>(g_pageFlipClickPos.x);
    float centerY = static_cast<float>(g_pageFlipClickPos.y);
    
    // 绘制圆形背景
    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(indicatorAlpha * 20 / 100, r, g, b));
    graphics.FillEllipse(&bgBrush, 
        centerX - arrowSize / 2, centerY - arrowSize / 2,
        arrowSize, arrowSize);
    
    // 绘制箭头
    Gdiplus::Pen arrowPen(Gdiplus::Color(indicatorAlpha, r, g, b), 3.0f);
    arrowPen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);
    
    float arrowLen = arrowSize * 0.35f;
    float arrowWidth = arrowSize * 0.25f;
    
    if (g_pageFlipDirection == FLIP_UP) {
        // 向上箭头 ↑
        // 竖线
        graphics.DrawLine(&arrowPen, 
            centerX, centerY + arrowLen / 2,
            centerX, centerY - arrowLen / 2);
        // 左边斜线
        graphics.DrawLine(&arrowPen,
            centerX, centerY - arrowLen / 2,
            centerX - arrowWidth, centerY - arrowLen / 2 + arrowWidth);
        // 右边斜线
        graphics.DrawLine(&arrowPen,
            centerX, centerY - arrowLen / 2,
            centerX + arrowWidth, centerY - arrowLen / 2 + arrowWidth);
    } else if (g_pageFlipDirection == FLIP_DOWN) {
        // 向下箭头 ↓
        // 竖线
        graphics.DrawLine(&arrowPen,
            centerX, centerY - arrowLen / 2,
            centerX, centerY + arrowLen / 2);
        // 左边斜线
        graphics.DrawLine(&arrowPen,
            centerX, centerY + arrowLen / 2,
            centerX - arrowWidth, centerY + arrowLen / 2 - arrowWidth);
        // 右边斜线
        graphics.DrawLine(&arrowPen,
            centerX, centerY + arrowLen / 2,
            centerX + arrowWidth, centerY + arrowLen / 2 - arrowWidth);
    }
    
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
}

// 切换显示/隐藏
void ToggleVisibility() {
    g_visible = !g_visible;
    ShowWindow(g_hwnd, g_visible ? SW_SHOW : SW_HIDE);
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            // 注册全局热键
            RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_ALT, VK_UP);      // Ctrl+Alt+Up 向上滚动
            RegisterHotKey(hwnd, 2, MOD_CONTROL | MOD_ALT, VK_DOWN);    // Ctrl+Alt+Down 向下滚动
            RegisterHotKey(hwnd, 3, MOD_CONTROL | MOD_ALT, 'H');        // Ctrl+Alt+H 隐藏/显示
            RegisterHotKey(hwnd, 4, MOD_CONTROL | MOD_ALT, VK_ESCAPE);  // Ctrl+Alt+ESC 退出
            RegisterHotKey(hwnd, 5, MOD_CONTROL | MOD_SHIFT, 'L');      // Ctrl+Shift+L 切换锁定
            DragAcceptFiles(hwnd, TRUE);
            CreateOrUpdateTrayIcon(((LPCREATESTRUCT)lParam)->hInstance);
            // 启动 Ctrl 键检测定时器
            SetTimer(hwnd, TIMER_ID_CTRL_CHECK, 50, NULL);
            // 启动鼠标状态检测定时器
            SetTimer(hwnd, TIMER_ID_MOUSE_CHECK, 50, NULL);
            // 初始化穿透状态（确保启动时无穿透）
            UpdateClickThroughState();
            break;
            
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
            
        case WM_HOTKEY:
            switch (wParam) {
                case 1: Scroll(-3); break;      // 向上滚动3行
                case 2: Scroll(3); break;       // 向下滚动3行
                case 3: ToggleVisibility(); break;
                case 4: PostQuitMessage(0); break;
                case 5: ToggleLockState(); break; // 切换锁定状态
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_BG_SLIDER:
                    ShowSliderWindow(true);
                    break;
                case ID_TRAY_TEXT_ALPHA:
                    ShowSliderWindow(false);
                    break;
                case ID_TRAY_LINK_HIGHLIGHT:
                    g_linkHighlightEnabled = !g_linkHighlightEnabled;
                    RenderLayeredWindow();
                    SaveProgress();
                    break;
                case ID_TRAY_TEXT_COLOR:
                {
                    static COLORREF customColors[16] = {};
                    CHOOSECOLOR cc = {};
                    cc.lStructSize = sizeof(CHOOSECOLOR);
                    cc.hwndOwner = hwnd;
                    cc.rgbResult = g_textColor;
                    cc.lpCustColors = customColors;
                    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ENABLEHOOK;
                    cc.lpfnHook = ColorDialogHookProc;
                    
                    g_originalTextColor = g_textColor; // 保存原始颜色
                    
                    if (ChooseColor(&cc)) {
                        // 用户点击确定，保存颜色
                        g_textColor = cc.rgbResult;
                        RenderLayeredWindow();
                        SaveProgress();
                    } else {
                        // 用户点击取消，恢复原始颜色
                        g_textColor = g_originalTextColor;
                        RenderLayeredWindow();
                    }
                    break;
                }
                case ID_TRAY_LOCK_WINDOW:
                    ToggleLockState();
                    break;
                case ID_TRAY_RELOAD:
                    if (!g_currentFile.empty()) {
                        LoadFileAndReset(g_currentFile);
                    }
                    break;
                case ID_TRAY_OPEN_FILE:
                    OpenFileDialog(hwnd);
                    break;
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            break;

        case WM_LBUTTONDOWN:
        {
            // 获取客户区坐标
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // 检查是否点击了关闭按钮
            if (IsPointInRect(x, y, g_closeButtonRect)) {
                // 隐藏窗口到托盘
                ShowWindow(hwnd, SW_HIDE);
                g_visible = false;
                return 0;
            }
            
            // 检查是否点击了锁定图标
            if (IsPointInRect(x, y, g_lockIconRect)) {
                ToggleLockState();
                return 0;
            }
            
            // 锁定状态下，点击空白区域不响应（实现"点击穿透"）
            if (g_isLocked && !g_ctrlPressed) {
                return 0;
            }
            
            if (g_isLocked && g_ctrlPressed && !g_prevForeground) {
                HWND fg = GetForegroundWindow();
                if (fg && fg != g_hwnd) {
                    g_prevForeground = fg;
                }
            }

            // 否则开始拖动窗口
            SetCapture(hwnd);
            g_isDragging = true;
            g_hasMovedDuringDrag = false;
            GetCursorPos(&g_dragStart);
            RECT windowRect;
            GetWindowRect(hwnd, &windowRect);
            g_windowStart.x = windowRect.left;
            g_windowStart.y = windowRect.top;
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            // 获取客户区坐标
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // 检查鼠标是否悬停在关闭按钮上
            bool wasCloseHovered = g_closeButtonHovered;
            g_closeButtonHovered = IsPointInRect(x, y, g_closeButtonRect);
            
            // 检查鼠标是否悬停在锁定图标上
            bool wasLockHovered = g_lockIconHovered;
            g_lockIconHovered = IsPointInRect(x, y, g_lockIconRect);
            
            // 如果悬停状态改变，重新绘制
            if (wasCloseHovered != g_closeButtonHovered || wasLockHovered != g_lockIconHovered) {
                RenderLayeredWindow();
            }
            
            bool hoverChanged = false;
            int hoveredDisplayIdx = -1;
            LinkRange hoveredRange;
            if (HitTestLink(x, y, hoveredRange.url, &hoveredDisplayIdx, &hoveredRange)) {
                if (!g_hoveredLinkActive ||
                    g_hoveredLinkDisplayIndex != hoveredDisplayIdx ||
                    g_hoveredLinkRange.startChar != hoveredRange.startChar ||
                    g_hoveredLinkRange.length != hoveredRange.length) {
                    g_hoveredLinkActive = true;
                    g_hoveredLinkDisplayIndex = hoveredDisplayIdx;
                    g_hoveredLinkRange = hoveredRange;
                    hoverChanged = true;
                }
            } else if (g_hoveredLinkActive) {
                g_hoveredLinkActive = false;
                g_hoveredLinkDisplayIndex = -1;
                g_hoveredLinkRange = LinkRange();
                hoverChanged = true;
            }
            if (hoverChanged) {
                RenderLayeredWindow();
            }
            
            // 处理窗口拖动
            if (g_isDragging && (wParam & MK_LBUTTON)) {
                POINT current;
                GetCursorPos(&current);
                int dx = current.x - g_dragStart.x;
                int dy = current.y - g_dragStart.y;
                if (!g_hasMovedDuringDrag) {
                    if (std::abs(dx) < DRAG_THRESHOLD && std::abs(dy) < DRAG_THRESHOLD) {
                        return 0;
                    }
                    g_hasMovedDuringDrag = true;
                }
                SetWindowPos(hwnd, NULL, g_windowStart.x + dx, g_windowStart.y + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                RenderLayeredWindow();
                return 0;
            }
            break;
        }

        case WM_LBUTTONUP:
            if (g_isDragging) {
                ReleaseCapture();
                g_isDragging = false;
                bool moved = g_hasMovedDuringDrag;
                g_hasMovedDuringDrag = false;
                if (!moved) {
                    if (TryHandleLinkClick(LOWORD(lParam), HIWORD(lParam))) {
                        if (g_isLocked && !g_ctrlPressed) {
                            RestorePreviousForeground();
                        }
                        return 0;
                    }
                }
                if (g_isLocked && !g_ctrlPressed) {
                    RestorePreviousForeground();
                }
                return 0;
            }
            break;

        case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                Scroll(delta > 0 ? -1 : 1);
            }
            break;
        
        case WM_RBUTTONDOWN:
        {
            // 获取客户区坐标
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // 获取窗口高度，判断点击在上半部分还是下半部分
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int windowHeight = clientRect.bottom - clientRect.top;
            int midHeight = windowHeight / 2;
            
            if (y < midHeight) {
                // 点击上半部分：向上翻页
                ScrollHalfPage(true);
                StartPageFlipAnimation(FLIP_UP, x, y);
            } else {
                // 点击下半部分：向下翻页
                ScrollHalfPage(false);
                StartPageFlipAnimation(FLIP_DOWN, x, y);
            }
            break;
        }
        
        case WM_TIMER:
        {
            if (wParam == TIMER_ID_PAGE_INDICATOR) {
                // 定时更新翻页指示器动画
                if (g_pageFlipDirection != FLIP_NONE) {
                    RenderLayeredWindow();
                } else {
                    // 动画结束，停止定时器
                    KillTimer(hwnd, TIMER_ID_PAGE_INDICATOR);
                }
            } else if (wParam == TIMER_ID_CTRL_CHECK) {
                // 检查 Ctrl 键状态（用于临时交互）
                CheckCtrlKeyState();
            } else if (wParam == TIMER_ID_MOUSE_CHECK) {
                // 检查鼠标状态（用于锁定模式右键翻页、按钮交互）
                CheckMouseState();
            }
            break;
        }
        
        case WM_NCHITTEST:
        {
            POINT ptScreen = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &ptScreen);
            bool inClose = IsPointInRect(ptScreen.x, ptScreen.y, g_closeButtonRect);
            bool inLock = IsPointInRect(ptScreen.x, ptScreen.y, g_lockIconRect);

            if (g_isLocked && !g_ctrlPressed && !g_rightButtonPressed && !inClose && !inLock) {
                DebugLog(L"[HitTest] passthrough (%d,%d)\n", ptScreen.x, ptScreen.y);
                return HTTRANSPARENT;
            }

            DebugLog(L"[HitTest] client (%d,%d) locked=%d ctrl=%d right=%d\n",
                     ptScreen.x, ptScreen.y,
                     g_isLocked ? 1 : 0,
                     g_ctrlPressed ? 1 : 0,
                     g_rightButtonPressed ? 1 : 0);
            return HTCLIENT;
        }

        case WM_TRAYICON:
            if (lParam == WM_LBUTTONUP) {
                // 左键单击：显示/隐藏窗口
                if (g_visible) {
                    ShowWindow(hwnd, SW_HIDE);
                    g_visible = false;
                } else {
                    ShowWindow(hwnd, SW_SHOW);
                    g_visible = true;
                    SetForegroundWindow(hwnd);
                }
            } else if (lParam == WM_RBUTTONUP) {
                // 右键单击：显示菜单
                HMENU menu = CreateTrayMenu();
                if (menu) {
                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                    DestroyMenu(menu);
                }
            }
            break;

        case WM_DROPFILES:
            {
                HDROP hDrop = reinterpret_cast<HDROP>(wParam);
                UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
                if (fileCount > 0) {
                    UINT charCount = DragQueryFileW(hDrop, 0, NULL, 0) + 1;
                    std::wstring path(charCount, L'\0');
                    DragQueryFileW(hDrop, 0, &path[0], charCount);
                    path.resize(wcslen(path.c_str()));
                    LoadFileAndReset(path);
                }
                DragFinish(hDrop);
            }
            break;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);
                RenderLayeredWindow();
            }
            break;
            
        case WM_DESTROY:
            UnregisterHotKey(hwnd, 1);
            UnregisterHotKey(hwnd, 2);
            UnregisterHotKey(hwnd, 3);
            UnregisterHotKey(hwnd, 4);
            UnregisterHotKey(hwnd, 5);
            DragAcceptFiles(hwnd, FALSE);
            DestroySliderWindow();
            RemoveTrayIcon();
            // 清理所有定时器
            KillTimer(hwnd, TIMER_ID_PAGE_INDICATOR);
            KillTimer(hwnd, TIMER_ID_CTRL_CHECK);
            KillTimer(hwnd, TIMER_ID_MOUSE_CHECK);
            if (g_gdiplusToken) {
                Gdiplus::GdiplusShutdown(g_gdiplusToken);
                g_gdiplusToken = 0;
            }
            PostQuitMessage(0);
            break;
            
        default:
            if (g_taskbarCreatedMsg != 0 && msg == g_taskbarCreatedMsg) {
                CreateOrUpdateTrayIcon(GetModuleHandle(NULL));
                return 0;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 防止多实例运行
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"TransparentMDReaderMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"程序已经在运行中！\n请检查系统托盘。", L"提示", MB_OK | MB_ICONINFORMATION);
        if (hMutex) {
            CloseHandle(hMutex);
        }
        return 0;
    }

    if (Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusInput, NULL) != Gdiplus::Ok) {
        MessageBox(NULL, L"无法初始化 GDI+", L"错误", MB_OK | MB_ICONERROR);
        if (hMutex) {
            CloseHandle(hMutex);
        }
        return 1;
    }

    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    g_taskbarCreatedMsg = RegisterWindowMessage(L"TaskbarCreated");

    InitializePaths();

    // 初始化 SQLite 历史数据库
    InitHistoryDB();

    // 注册窗口类
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"TransparentMDReader";
    RegisterClassEx(&wc);
    
    // 创建透明置顶窗口
    g_hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,  // 置顶 + 分层 + 工具窗口（不显示任务栏）
        L"TransparentMDReader",
        L"MD Reader",
        WS_POPUP,  // 无边框
        100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hwnd) {
        MessageBox(NULL, L"窗口创建失败", L"错误", MB_OK | MB_ICONERROR);
        if (g_gdiplusToken) {
            Gdiplus::GdiplusShutdown(g_gdiplusToken);
            g_gdiplusToken = 0;
        }
        if (hMutex) {
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
        }
        return 1;
    }
    
    // 先加载上次的进度
    LoadProgress();
    
    // 如果有命令行参数，使用第一个参数作为文件路径（覆盖保存的进度）
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc > 1) {
        LoadFileAndReset(argv[1]);
    } else if (g_currentFile.empty() || g_lines.empty()) {
        // 如果没有保存的文件或加载失败，加载默认文件
        LoadFileAndReset(L"test.md");
    } else {
        // 窗口已创建，对已加载的文本进行换行处理
        WrapTextLines();
    }
    LocalFree(argv);
    
    // 应用加载的锁定状态（在显示窗口前）
    UpdateClickThroughState();
    
    ShowWindow(g_hwnd, SW_SHOW);
    RenderLayeredWindow();
    
    // 确保托盘图标被创建（在窗口显示后再次尝试）
    CreateOrUpdateTrayIcon(hInstance);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // 关闭 SQLite 数据库
    CloseHistoryDB();
    
    if (g_gdiplusToken) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }

    // 释放Mutex
    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    return (int)msg.wParam;
}




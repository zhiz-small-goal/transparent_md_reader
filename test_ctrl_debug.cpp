// 临时调试代码片段
// 在 CheckCtrlKeyState() 中添加：

void CheckCtrlKeyState() {
    bool ctrlNowPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    
    // 调试输出
    if (ctrlNowPressed != g_ctrlPressed) {
        wchar_t debug[256];
        swprintf_s(debug, L"Ctrl state changed: %d -> %d, isLocked=%d", 
                   g_ctrlPressed, ctrlNowPressed, g_isLocked);
        OutputDebugString(debug);
        
        // ... 原有代码 ...
        
        g_ctrlPressed = ctrlNowPressed;

        if (g_isLocked) {
            LONG exStyle = GetWindowLong(g_hwnd, GWL_EXSTYLE);
            bool hasTransparent = (exStyle & WS_EX_TRANSPARENT) != 0;
            
            swprintf_s(debug, L"Before UpdateClickThroughState: WS_EX_TRANSPARENT=%d", hasTransparent);
            OutputDebugString(debug);
            
            UpdateClickThroughState();
            
            exStyle = GetWindowLong(g_hwnd, GWL_EXSTYLE);
            hasTransparent = (exStyle & WS_EX_TRANSPARENT) != 0;
            
            swprintf_s(debug, L"After UpdateClickThroughState: WS_EX_TRANSPARENT=%d", hasTransparent);
            OutputDebugString(debug);
            
            RenderLayeredWindow();
        }
    }
}


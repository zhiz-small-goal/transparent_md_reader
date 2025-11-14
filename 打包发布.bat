@echo off
chcp 65001 >nul
cls
echo.
echo ========================================
echo   透明 Markdown 阅读器 v2.0
echo   快速打包脚本
echo ========================================
echo.

REM 检查 exe 是否存在
if not exist "md_reader.exe" (
    echo [错误] 找不到 md_reader.exe！
    echo 请先运行 build.bat 编译程序
    echo.
    pause
    exit /b 1
)

REM 设置变量
set FOLDER=透明MD阅读器_v2.0
set DEST=release\%FOLDER%

echo [步骤 1/4] 创建发布目录...
if exist "release" rmdir /s /q "release" 2>nul
mkdir "%DEST%" 2>nul

echo [步骤 2/4] 复制主程序...
copy "md_reader.exe" "%DEST%\" >nul
if errorlevel 1 (
    echo 复制失败！请检查文件权限
    pause
    exit /b 1
)

echo [步骤 3/4] 复制文档...
if exist "使用手册.html" (
    copy "使用手册.html" "%DEST%\📖 使用手册.html" >nul
    echo   ✓ 使用手册
) else (
    echo   ✗ 未找到使用手册.html
)

if exist "README.md" (
    copy "README.md" "%DEST%\" >nul
    echo   ✓ README
)

if exist "test.md" (
    copy "test.md" "%DEST%\示例文档.md" >nul
    echo   ✓ 示例文档
)

echo [步骤 4/4] 创建快速说明...
(
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo   透明 Markdown 阅读器 v2.0.0
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.
echo 【5秒快速上手】
echo.
echo 1. 双击 md_reader.exe 启动程序
echo 2. 拖动 .md 文件到窗口上打开
echo 3. 右键托盘图标调整透明度
echo.
echo ─────────────────────────────────────
echo 【核心功能】
echo ─────────────────────────────────────
echo.
echo 🔒 锁定模式：点击右上角锁定图标
echo    - 窗口穿透（左键点击穿过窗口）
echo    - 右键翻页仍然有效！
echo    - 按住 Ctrl 可临时拖动
echo.
echo 📚 智能记忆：每个文件独立保存
echo    - 阅读位置
echo    - 锁定状态
echo    - 自动恢复
echo.
echo 🖱️ 翻页操作：
echo    - 滚轮：逐行滚动
echo    - 右键上半部分：向上翻半页
echo    - 右键下半部分：向下翻半页
echo.
echo 🎨 自定义：右键托盘图标
echo    - 背景透明度（推荐设为0）
echo    - 文字透明度（推荐200-255）
echo    - 文字颜色
echo.
echo ─────────────────────────────────────
echo 【快捷键】
echo ─────────────────────────────────────
echo.
echo Ctrl + Alt + ↑/↓  滚动
echo Ctrl + Alt + H    显示/隐藏
echo Ctrl + Alt + Esc  退出
echo Ctrl + Shift + L  切换锁定
echo.
echo ─────────────────────────────────────
echo 【详细说明】
echo ─────────────────────────────────────
echo.
echo 双击 "📖 使用手册.html" 查看完整图文教程！
echo.
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo   免安装 ^| 解压即用 ^| 完全本地运行
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
) > "%DEST%\快速上手指南.txt"

echo.
echo ========================================
echo   ✓ 打包完成！
echo ========================================
echo.
echo 📁 发布包位置：
echo    %DEST%
echo.
echo 📦 包含文件：
dir /b "%DEST%" 2>nul | findstr /v "^$"
echo.
echo 💡 下一步：
echo    1. 将 %FOLDER% 文件夹压缩为 ZIP
echo    2. 重命名为：透明MD阅读器_v2.0.zip
echo    3. 可以发布了！
echo.
echo 🎯 发布亮点：
echo    ✓ SQLite 历史记录（每个文件独立保存）
echo    ✓ 锁定模式右键翻页仍可用
echo    ✓ 完整的 HTML 使用手册
echo    ✓ 免安装，解压即用
echo.
pause

